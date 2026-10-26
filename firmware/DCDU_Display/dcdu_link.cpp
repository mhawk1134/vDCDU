// vDCDU display firmware. Copyright (c) 2026 SkySense Labs (https://www.skysenselabs.com)
// PolyForm Noncommercial License 1.0.0: see firmware/LICENSE.md.
// The link to the bridge: USB cable and/or Wi-Fi WebSocket.
//
// USB: newline-delimited JSON over the USB serial port, the same messages as the
// WebSocket. The bridge finds the display by itself and pings every 5 s. Lines that
// don't start with '{' (our own debug prints) are ignored by both sides.
// Wi-Fi: WebSocket to the saved sim PC address; if that doesn't answer, the bridge
// is looked up with mDNS (it advertises _dcdu._tcp).
// While USB is up it is used for the screen and keys; otherwise the WebSocket is.

#include "dcdu.h"
#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ESPmDNS.h>
#include "mdns.h"

static const uint32_t USB_TIMEOUT_MS = 10000;    // no frame from the bridge for this long
static const uint32_t SEARCH_AFTER_MS = 15000;   // saved address not answering: try mDNS

static WebSocketsClient ws;
static bool wsConnected = false;
static bool wsBegun = false;
static char wsHost[64];
static uint16_t wsPort;
static uint32_t wsSince;                 // when we started trying wsHost

static char line[8192];
static size_t lineLen = 0;
static bool lineSkip = false;            // line too long: drop it
static uint32_t usbRx = 0;               // last frame from the bridge over USB
static bool usbWas = false;

static bool mdnsStarted = false;
static mdns_search_once_t *search = nullptr;
static uint32_t lastSearch = 0;

bool usbUp() { return usbRx && millis() - usbRx < USB_TIMEOUT_MS; }
bool wsUp() { return wsConnected; }
const char *bridgeTarget() { return wsBegun ? wsHost : ""; }

void sendTo(From to, JsonDocument &doc) {
  String out;
  serializeJson(doc, out);
  if (to == FROM_USB) {
    out += '\n';
    Serial.write((const uint8_t *)out.c_str(), out.length());
  } else if (to == FROM_WS && wsConnected) {
    ws.sendTXT(out);
  }
}

static const char *stateName() {
  switch (wifiState()) {
    case WS_JOINING: return "joining";
    case WS_UP:      return "up";
    case WS_FAILED:  return "failed";
    default:         return "off";
  }
}

// Who we are and how we're set up; the web page shows this under Settings > Display.
static void hello(From to) {
  JsonDocument d;
  d["type"] = "hello";
  d["name"] = DEVICE_NAME;
  d["id"] = deviceId;
  d["fw"] = FW_VERSION;
  d["link"] = to == FROM_USB ? "usb" : "wifi";
  d["mode"] = cfg.mode;
  d["wifi"] = stateName();
  d["ssid"] = cfg.ssid;                  // saved network (in use only in Wi-Fi mode)
  d["ip"] = wifiState() == WS_UP ? WiFi.localIP().toString() : "";
  d["host"] = cfg.host;
  d["port"] = cfg.port;
  d["board"] = boardName;
  fittedKeys(d["keys"].to<JsonArray>());   // so the web page can show the wiring
  sendTo(to, d);
}

void sendKey(const char *key) {
  wake();
  JsonDocument d;
  d["type"] = "key";
  d["key"] = key;
  if (usbUp()) sendTo(FROM_USB, d);
  else if (wsConnected) sendTo(FROM_WS, d);
}

static void handle(JsonDocument &doc, From from) {
  const char *type = doc["type"] | "";
  if (from == FROM_USB) {
    bool was = usbUp();
    usbRx = millis();
    if (!was) hello(FROM_USB);
  }
  bool show = from == FROM_USB || !usbUp();
  if (!strcmp(type, "screen")) {
    if (show) takeScreen(doc["s"]);
  } else if (!strcmp(type, "state")) {
    if (show) takeScreen(doc["screen"]);
  } else if (!strcmp(type, "wifi_scan")) {
    wifiScan(from);
  } else if (!strcmp(type, "wifi_set")) {
    wifiJoin(doc["ssid"] | "", doc["pass"] | "", from);
  } else if (!strcmp(type, "set_mode")) {
    wifiSetMode(doc["mode"] | "");
  } else if (!strcmp(type, "set_host")) {
    wifiSetHost(doc["host"] | "", doc["port"] | BRIDGE_PORT);
  } else if (!strcmp(type, "wifi_forget")) {
    wifiForget();
  }
}

// ---------------------------------------------------------------------- USB --
static void readUsb() {
  int budget = 4096;                     // keep the screen and keys responsive
  while (budget-- > 0 && Serial.available()) {
    uint8_t c = Serial.read();
    if (improvByte(c, lineLen == 0 && !lineSkip)) continue;
    if (c == '\n' || c == '\r') {
      if (lineLen && !lineSkip && line[0] == '{') {
        JsonDocument doc;
        if (!deserializeJson(doc, (const char *)line, lineLen)) handle(doc, FROM_USB);
      }
      lineLen = 0;
      lineSkip = false;
    } else if (lineLen < sizeof line) {
      line[lineLen++] = c;
    } else {
      lineSkip = true;
    }
  }
}

// ---------------------------------------------------------------- WebSocket --
static void onWs(WStype_t type, uint8_t *payload, size_t len) {
  switch (type) {
    case WStype_CONNECTED:
      wsConnected = true;
      hello(FROM_WS);
      dirty = true;
      Serial.printf("bridge connected: %s:%u\n", wsHost, wsPort);
      break;
    case WStype_DISCONNECTED:
      if (wsConnected) { dirty = true; Serial.println("bridge disconnected"); }
      wsConnected = false;
      break;
    case WStype_TEXT: {
      JsonDocument doc;
      if (!deserializeJson(doc, (const char *)payload, len)) handle(doc, FROM_WS);
      break;
    }
    default:
      break;
  }
}

// Point the WebSocket at host:port ("" = stop).
static void target(const char *host, uint16_t port) {
  if (wsBegun && !strcmp(host, wsHost) && port == wsPort) return;
  if (wsBegun) {
    ws.disconnect();
    wsBegun = false;
    wsConnected = false;
    dirty = true;
  }
  if (!*host) return;
  strlcpy(wsHost, host, sizeof wsHost);
  wsPort = port;
  ws.begin(wsHost, wsPort, "/ws");
  ws.onEvent(onWs);
  ws.setReconnectInterval(3000);
  ws.enableHeartbeat(15000, 3000, 2);
  wsBegun = true;
  wsSince = millis();
  dirty = true;
}

void linkHostChanged() {
  if (!wifiMode()) target("", 0);
  else if (cfg.host[0]) target(cfg.host, cfg.port);
}

// ------------------------------------------------------------------- mDNS --
static void useFound(mdns_result_t *res) {
  for (mdns_result_t *r = res; r; r = r->next) {
    uint16_t port = r->port ? r->port : BRIDGE_PORT;
    for (mdns_ip_addr_t *a = r->addr; a; a = a->next) {
      if (a->addr.type != ESP_IPADDR_TYPE_V4) continue;
      String ip = IPAddress(a->addr.u_addr.ip4.addr).toString();
      Serial.printf("bridge found with mDNS: %s:%u\n", ip.c_str(), port);
      wifiSetHost(ip.c_str(), port);
      return;
    }
    if (r->hostname) {                   // address not in the answer: ask for it
      IPAddress ip = MDNS.queryHost(r->hostname, 1000);
      if (ip != IPAddress()) {
        wifiSetHost(ip.toString().c_str(), port);
        return;
      }
    }
  }
}

static void discover() {
  if (search) {
    mdns_result_t *res = nullptr;
    if (!mdns_query_async_get_results(search, 0, &res, nullptr)) return;   // still asking
    useFound(res);
    if (res) mdns_query_results_free(res);
    mdns_query_async_delete(search);
    search = nullptr;
    return;
  }
  if (millis() - lastSearch < SEARCH_AFTER_MS) return;
  if (wsBegun && millis() - wsSince < SEARCH_AFTER_MS) return;   // saved address first
  if (!mdnsStarted) mdnsStarted = MDNS.begin(DEVICE_NAME);
  if (!mdnsStarted) return;
  lastSearch = millis();
  search = mdns_query_async_new(NULL, "_dcdu", "_tcp", MDNS_TYPE_PTR, 3000, 4, NULL);
}

// ------------------------------------------------------------------- loop --
void linkSetup() {
  linkHostChanged();
}

void linkLoop() {
  readUsb();

  bool u = usbUp();
  if (u != usbWas) {
    usbWas = u;
    dirty = true;
    Serial.println(u ? "bridge connected over USB" : "USB link lost");
    if (!u && wsConnected) ws.disconnect();   // reconnects and gets the current screen
  }

  if (wsBegun) ws.loop();
  if (wifiState() == WS_UP && !wsConnected) discover();

  // Tell the bridge who we are: over USB every 2 s until it answers, and on both
  // links whenever something changes (so the page shows the current state).
  static uint32_t lastHello = 0, lastCheck = 0;
  static String sig;
  if (!u && millis() - lastHello > 2000) {
    lastHello = millis();
    hello(FROM_USB);
  }
  if (millis() - lastCheck > 500) {
    lastCheck = millis();
    String now = String(cfg.mode) + "|" + cfg.ssid + "|" + cfg.host + "|" + cfg.port + "|" +
                 stateName() + "|" + (wifiState() == WS_UP ? WiFi.localIP().toString() : "");
    if (now != sig) {
      sig = now;
      if (u) hello(FROM_USB);
      if (wsConnected) hello(FROM_WS);
      dirty = true;
    }
  }
}
