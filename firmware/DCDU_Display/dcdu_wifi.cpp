// vDCDU display firmware. Copyright (c) 2026 mhawk1134 (https://github.com/mhawk1134/vDCDU)
// PolyForm Noncommercial License 1.0.0: see firmware/LICENSE.md.
// Saved settings, Wi-Fi and Improv.
//
// Settings live in NVS (flash), so they survive power-off and firmware updates. They
// are set from the DCDU web page (sent by the bridge over USB) or by Improv Wi-Fi.
// config.h values are used only until the first change from the page.
//
// Modes: "usb"  - Wi-Fi off; the bridge talks to us over the USB cable.
//        "wifi" - join cfg.ssid and connect to the bridge over the network.
// The USB link works in both modes, so a display can always be set up again by
// plugging it into the sim PC.

#include "dcdu.h"
#include <WiFi.h>
#include <Preferences.h>
#include "esp_mac.h"

Settings cfg;
char deviceId[16];

static Preferences prefs;
static WifiState state = WS_OFF;
static uint32_t downSince = 0;           // Wi-Fi mode, not connected since (0 = connected)
static volatile uint8_t lastReason = 0;  // why the last Wi-Fi attempt failed

static bool joining = false;             // trying a network sent by the page or Improv
static From joinFrom;
static uint32_t joinStart;
static char joinSsid[33], joinPass[65];
static const uint32_t JOIN_TIMEOUT_MS = 15000;
static const uint32_t FAILED_AFTER_MS = 30000;

static bool scanning = false;
static bool scanFor[3];                  // who asked (FROM_USB / FROM_WS / FROM_IMPROV)

static void improvJoined(bool ok);
static void improvNetwork(const char *ssid, int rssi, bool secure);
static void improvScanDone();
static void improvProvisioning();

// --------------------------------------------------------------- settings --
static void loadSettings() {
  memset(&cfg, 0, sizeof cfg);
  prefs.begin("dcdu", false);
  bool saved = prefs.isKey("mode");
  if (saved) {
    prefs.getString("mode", cfg.mode, sizeof cfg.mode);
    prefs.getString("ssid", cfg.ssid, sizeof cfg.ssid);
    prefs.getString("pass", cfg.pass, sizeof cfg.pass);
    prefs.getString("host", cfg.host, sizeof cfg.host);
    cfg.port = prefs.getUShort("port", BRIDGE_PORT);
  }
  prefs.end();
  if (!saved) {                          // never changed from the page: config.h defaults
    bool haveWifi = strlen(WIFI_SSID) && strcmp(WIFI_SSID, "your-ssid");
    strlcpy(cfg.mode, haveWifi ? "wifi" : "usb", sizeof cfg.mode);
    strlcpy(cfg.ssid, haveWifi ? WIFI_SSID : "", sizeof cfg.ssid);
    strlcpy(cfg.pass, haveWifi ? WIFI_PASSWORD : "", sizeof cfg.pass);
    strlcpy(cfg.host, BRIDGE_HOST, sizeof cfg.host);
    cfg.port = BRIDGE_PORT;
  }
  if (!cfg.port) cfg.port = BRIDGE_PORT;
  if (strcmp(cfg.mode, "wifi") || !cfg.ssid[0]) strlcpy(cfg.mode, "usb", sizeof cfg.mode);
}

static void saveSettings() {
  prefs.begin("dcdu", false);
  prefs.putString("mode", cfg.mode);
  prefs.putString("ssid", cfg.ssid);
  prefs.putString("pass", cfg.pass);
  prefs.putString("host", cfg.host);
  prefs.putUShort("port", cfg.port);
  prefs.end();
}

// ------------------------------------------------------------------- Wi-Fi --
bool wifiMode() { return !strcmp(cfg.mode, "wifi"); }
WifiState wifiState() { return state; }

static void onWifiEvent(arduino_event_id_t, arduino_event_info_t info) {
  lastReason = info.wifi_sta_disconnected.reason;
}

static void startStation(const char *ssid, const char *pass) {
  WiFi.setHostname(DEVICE_NAME);         // must come before WiFi.mode() on ESP32 core 3.x
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);
  lastReason = 0;
  WiFi.begin(ssid, pass);
}

static void wifiOff() {
  if (WiFi.getMode() != WIFI_OFF) {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
  }
}

// Start (or stop) Wi-Fi to match the saved settings.
static void applyMode() {
  joining = false;
  if (wifiMode()) {
    startStation(cfg.ssid, cfg.pass);
    state = WS_JOINING;
    downSince = millis();
  } else {
    if (!scanning) wifiOff();
    state = WS_OFF;
  }
  dirty = true;
}

static const char *reasonText() {
  switch (lastReason) {
    case WIFI_REASON_NO_AP_FOUND:
    case WIFI_REASON_NO_AP_FOUND_W_COMPATIBLE_SECURITY:
    case WIFI_REASON_NO_AP_FOUND_IN_AUTHMODE_THRESHOLD:
    case WIFI_REASON_NO_AP_FOUND_IN_RSSI_THRESHOLD:
      return "network not found (the display can only use 2.4 GHz Wi-Fi)";
    case WIFI_REASON_AUTH_FAIL:
    case WIFI_REASON_AUTH_EXPIRE:
    case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
    case WIFI_REASON_HANDSHAKE_TIMEOUT:
      return "wrong password";
    default:
      return "could not connect";
  }
}

// Try a network from the page or Improv. Only saved once it works; if it doesn't,
// the display goes back to what it had.
void wifiJoin(const char *ssid, const char *pass, From from) {
  strlcpy(joinSsid, ssid ? ssid : "", sizeof joinSsid);
  strlcpy(joinPass, pass ? pass : "", sizeof joinPass);
  joinFrom = from;
  if (!joinSsid[0]) {
    lastReason = 0;
    joinStart = millis() - JOIN_TIMEOUT_MS;          // fails on the next loop
  } else {
    joinStart = millis();
    if (WiFi.getMode() != WIFI_OFF) WiFi.disconnect();
    startStation(joinSsid, joinPass);
  }
  joining = true;
  state = WS_JOINING;
  dirty = true;
  if (from == FROM_IMPROV) improvProvisioning();
  Serial.printf("trying Wi-Fi %s\n", joinSsid);
}

static void finishJoin(bool ok) {
  const char *err = ok ? "" : (joinSsid[0] ? reasonText() : "no network name");
  if (ok) {
    strlcpy(cfg.mode, "wifi", sizeof cfg.mode);
    strlcpy(cfg.ssid, joinSsid, sizeof cfg.ssid);
    strlcpy(cfg.pass, joinPass, sizeof cfg.pass);
    saveSettings();
    joining = false;
    state = WS_UP;
    downSince = 0;
    linkHostChanged();
  } else {
    applyMode();                         // back to the saved network, or Wi-Fi off
  }
  Serial.printf("Wi-Fi %s: %s\n", joinSsid, ok ? "connected" : err);
  if (joinFrom == FROM_IMPROV) {
    improvJoined(ok);
  } else {
    JsonDocument d;
    d["type"] = "wifi_result";
    d["ok"] = ok;
    d["ssid"] = joinSsid;
    d["ip"] = ok ? WiFi.localIP().toString() : "";
    d["err"] = err;
    sendTo(joinFrom, d);
  }
  memset(joinPass, 0, sizeof joinPass);
  dirty = true;
}

void wifiSetMode(const char *mode) {
  bool wifi = !strcmp(mode, "wifi");
  if (wifi && !cfg.ssid[0]) return;      // no network saved yet: the page sends wifi_set
  if (!strcmp(cfg.mode, wifi ? "wifi" : "usb")) return;
  strlcpy(cfg.mode, wifi ? "wifi" : "usb", sizeof cfg.mode);
  saveSettings();
  applyMode();
  linkHostChanged();
}

void wifiSetHost(const char *host, uint16_t port) {
  if (!port) port = BRIDGE_PORT;
  if (!strcmp(host, cfg.host) && port == cfg.port) return;
  strlcpy(cfg.host, host, sizeof cfg.host);
  cfg.port = port;
  saveSettings();
  linkHostChanged();
}

void wifiForget() {
  cfg.ssid[0] = cfg.pass[0] = cfg.host[0] = 0;
  strlcpy(cfg.mode, "usb", sizeof cfg.mode);
  saveSettings();                        // saved "usb" also stops config.h defaults coming back
  applyMode();
  linkHostChanged();
}

// --------------------------------------------------------------------- scan --
void wifiScan(From from) {
  scanFor[from] = true;
  if (scanning) return;
  if (WiFi.getMode() == WIFI_OFF) {
    WiFi.setHostname(DEVICE_NAME);
    WiFi.mode(WIFI_STA);
  }
  WiFi.scanNetworks(true);               // async; wifiLoop() picks up the result (or failure)
  scanning = true;
}

struct Net {
  String ssid;
  int rssi;
  bool secure;
};

static void scanDone(int n) {
  Net nets[20];
  int count = 0;
  for (int i = 0; i < n; i++) {
    String s = WiFi.SSID(i);
    if (!s.length()) continue;           // hidden network
    int rssi = WiFi.RSSI(i);
    int j = 0;
    while (j < count && nets[j].ssid != s) j++;
    if (j < count) {                     // same network on several access points
      if (rssi > nets[j].rssi) nets[j].rssi = rssi;
      continue;
    }
    if (count == 20) continue;
    nets[count++] = {s, rssi, WiFi.encryptionType(i) != WIFI_AUTH_OPEN};
  }
  WiFi.scanDelete();
  for (int i = 1; i < count; i++)        // strongest first
    for (int j = i; j > 0 && nets[j].rssi > nets[j - 1].rssi; j--) {
      Net t = nets[j]; nets[j] = nets[j - 1]; nets[j - 1] = t;
    }

  for (int f = FROM_USB; f <= FROM_WS; f++) {
    if (!scanFor[f]) continue;
    JsonDocument d;
    d["type"] = "wifi_scan_result";
    d["ok"] = n >= 0;
    JsonArray list = d["networks"].to<JsonArray>();
    for (int i = 0; i < count; i++) {
      JsonObject o = list.add<JsonObject>();
      o["ssid"] = nets[i].ssid;
      o["rssi"] = nets[i].rssi;
      o["secure"] = nets[i].secure;
    }
    sendTo((From)f, d);
  }
  if (scanFor[FROM_IMPROV]) {
    for (int i = 0; i < count; i++) improvNetwork(nets[i].ssid.c_str(), nets[i].rssi, nets[i].secure);
    improvScanDone();
  }
  memset(scanFor, 0, sizeof scanFor);
  scanning = false;
  if (!wifiMode() && !joining) wifiOff();
}

// --------------------------------------------------------------------- loop --
void wifiSetup() {
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  snprintf(deviceId, sizeof deviceId, "dcdu-%02x%02x%02x", mac[3], mac[4], mac[5]);
  WiFi.onEvent(onWifiEvent, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  loadSettings();
  Serial.printf("%s mode %s%s%s\n", deviceId, cfg.mode, wifiMode() ? ", Wi-Fi " : "",
                wifiMode() ? cfg.ssid : "");
  applyMode();
}

void wifiLoop() {
  if (scanning) {
    int n = WiFi.scanComplete();
    if (n != WIFI_SCAN_RUNNING) scanDone(n);
  }
  bool up = WiFi.status() == WL_CONNECTED;
  if (joining) {
    if (up && joinSsid[0]) finishJoin(true);
    else if (millis() - joinStart > JOIN_TIMEOUT_MS) finishJoin(false);
    return;
  }
  WifiState s = WS_OFF;
  if (wifiMode()) {
    if (up) {
      downSince = 0;
      s = WS_UP;
    } else {
      if (!downSince) downSince = millis();
      s = millis() - downSince > FAILED_AFTER_MS ? WS_FAILED : WS_JOINING;
    }
  }
  if (s != state) {
    if (s == WS_UP) Serial.printf("Wi-Fi up: %s\n", WiFi.localIP().toString().c_str());
    state = s;
    dirty = true;
  }
}

// ------------------------------------------------------------------- Improv --
// Improv Wi-Fi over serial (https://www.improv-wifi.com/serial/): lets a flasher page
// such as ESP Web Tools set up Wi-Fi right after installing the firmware.
// Frame: "IMPROV" version(1) type length data... checksum (sum of all bytes before it).
enum { IMP_STATE = 1, IMP_ERROR = 2, IMP_RPC = 3, IMP_RESULT = 4 };
enum { IMP_READY = 2, IMP_PROVISIONING = 3, IMP_PROVISIONED = 4 };
enum { IMP_ERR_INVALID = 1, IMP_ERR_UNKNOWN = 2, IMP_ERR_CONNECT = 3 };
enum { CMD_WIFI = 1, CMD_STATE = 2, CMD_INFO = 3, CMD_SCAN = 4 };

static uint8_t imp[6 + 3 + 255 + 1];
static int impLen = 0;

static void impSend(uint8_t type, const uint8_t *data, uint8_t len) {
  uint8_t buf[6 + 3 + 255 + 2];
  memcpy(buf, "IMPROV", 6);
  buf[6] = 1;
  buf[7] = type;
  buf[8] = len;
  memcpy(buf + 9, data, len);
  uint8_t sum = 0;
  for (int i = 0; i < 9 + len; i++) sum += buf[i];
  buf[9 + len] = sum;
  buf[10 + len] = '\n';
  Serial.write(buf, 11 + len);
}

static void impState(uint8_t s) { impSend(IMP_STATE, &s, 1); }
static void impError(uint8_t e) { impSend(IMP_ERROR, &e, 1); }

// RPC result: command, length of the rest, then each string as length + bytes
static void impResult(uint8_t cmd, const char *const *strs, int n) {
  uint8_t d[255];
  int p = 2;
  for (int i = 0; i < n; i++) {
    size_t l = strlen(strs[i]);
    if (p + 1 + l > sizeof d) break;
    d[p++] = l;
    memcpy(d + p, strs[i], l);
    p += l;
  }
  d[0] = cmd;
  d[1] = p - 2;
  impSend(IMP_RESULT, d, p);
}

static uint8_t impCurrent() {
  return wifiMode() && WiFi.status() == WL_CONNECTED ? IMP_PROVISIONED : IMP_READY;
}

static void improvProvisioning() { impState(IMP_PROVISIONING); }

static void improvJoined(bool ok) {
  if (ok) {
    impState(IMP_PROVISIONED);
    impResult(CMD_WIFI, nullptr, 0);     // no web page on the display itself to point to
  } else {
    impError(IMP_ERR_CONNECT);
    impState(IMP_READY);
  }
}

static void improvNetwork(const char *ssid, int rssi, bool secure) {
  char r[8];
  snprintf(r, sizeof r, "%d", rssi);
  const char *s[] = {ssid, r, secure ? "YES" : "NO"};
  impResult(CMD_SCAN, s, 3);
}

static void improvScanDone() { impResult(CMD_SCAN, nullptr, 0); }

static void impFrame() {
  uint8_t type = imp[7], len = imp[8];
  const uint8_t *d = imp + 9;
  if (type != IMP_RPC || len < 2 || d[1] + 2 > len) { impError(IMP_ERR_INVALID); return; }
  uint8_t cmd = d[0], dlen = d[1];
  const uint8_t *a = d + 2;
  switch (cmd) {
    case CMD_WIFI: {
      uint8_t sl = a[0];
      if (1 + sl + 1 > dlen) { impError(IMP_ERR_INVALID); return; }
      uint8_t pl = a[1 + sl];
      if (1 + sl + 1 + pl > dlen) { impError(IMP_ERR_INVALID); return; }
      char ssid[33], pass[65];
      snprintf(ssid, sizeof ssid, "%.*s", sl, (const char *)a + 1);
      snprintf(pass, sizeof pass, "%.*s", pl, (const char *)a + 2 + sl);
      wifiJoin(ssid, pass, FROM_IMPROV);
      memset(pass, 0, sizeof pass);
      break;
    }
    case CMD_STATE: {
      uint8_t s = impCurrent();
      impState(s);
      if (s == IMP_PROVISIONED) impResult(CMD_STATE, nullptr, 0);
      break;
    }
    case CMD_INFO: {
      const char *s[] = {"DCDU", FW_VERSION, "ESP32-S3", deviceId};
      impResult(CMD_INFO, s, 4);
      break;
    }
    case CMD_SCAN:
      wifiScan(FROM_IMPROV);
      break;
    default:
      impError(IMP_ERR_UNKNOWN);
  }
}

// Fed every byte from USB while a frame may be starting (at the start of a line) or
// is in progress. Returns false for bytes that aren't Improv: they go to the JSON reader.
bool improvByte(uint8_t c, bool lineStart) {
  static const char H[] = "IMPROV";
  if (impLen == 0 && !lineStart) return false;
  if (impLen < 6) {
    if (c != (uint8_t)H[impLen]) { impLen = 0; return false; }
    imp[impLen++] = c;
    return true;
  }
  imp[impLen++] = c;
  if (impLen >= 9 && impLen == 9 + imp[8] + 1) {
    uint8_t sum = 0;
    for (int i = 0; i < impLen - 1; i++) sum += imp[i];
    if (imp[6] == 1 && sum == imp[impLen - 1]) impFrame();
    else impError(IMP_ERR_INVALID);
    impLen = 0;
  }
  return true;
}
