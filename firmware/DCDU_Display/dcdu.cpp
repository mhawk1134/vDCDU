// vDCDU display firmware. Copyright (c) 2026 SkySense Labs (https://www.skysenselabs.com)
// PolyForm Noncommercial License 1.0.0: see firmware/LICENSE.md.
#include <Arduino.h>
// DCDU display. Ready-made for the Guition JC4827W543C (ESP32-S3, 4.3" 480x272 NV3041A
// QSPI); other boards: board.h.
//
// Draws the A320-style DCDU screen that dcdu_bridge.py sends (the bridge holds
// all the DCDU logic, so this unit, the web page and any other display always
// show the same thing). Keys - physical buttons wired to GPIOs - go back to the
// bridge. Same 480x272 layout as the web page.
//
// Board: "ESP32S3 Dev Module", PSRAM "OPI PSRAM", Flash 4MB, Partition Scheme "Huge
// APP" (see DCDU_Display.ino). The link to the bridge (USB or Wi-Fi) is in
// dcdu_link.cpp, saved settings and Wi-Fi in dcdu_wifi.cpp. config.h is optional.

#include "dcdu.h"
#include <WiFi.h>
#include "board.h"
#include "FreeMonoBold12pt7b.h"
#include "FreeMonoBold9pt7b.h"

// ----------------------------------------------- optional key pins (config.h) --
// Wire each key between a GPIO and GND. Anything not defined is "not fitted".
#ifndef PIN_KEY_L1
#define PIN_KEY_L1 -1
#endif
#ifndef PIN_KEY_L2
#define PIN_KEY_L2 -1
#endif
#ifndef PIN_KEY_R1
#define PIN_KEY_R1 -1
#endif
#ifndef PIN_KEY_R2
#define PIN_KEY_R2 -1
#endif
#ifndef PIN_KEY_BRT
#define PIN_KEY_BRT -1
#endif
#ifndef PIN_KEY_DIM
#define PIN_KEY_DIM -1
#endif
#ifndef PIN_KEY_PGEM
#define PIN_KEY_PGEM -1
#endif
#ifndef PIN_KEY_PRINT
#define PIN_KEY_PRINT -1
#endif
// names from the first version of config.h still work
#ifndef PIN_KEY_MSGM
#ifdef PIN_BTN_MSG_PREV
#define PIN_KEY_MSGM PIN_BTN_MSG_PREV
#else
#define PIN_KEY_MSGM -1
#endif
#endif
#ifndef PIN_KEY_MSGP
#ifdef PIN_BTN_MSG_NEXT
#define PIN_KEY_MSGP PIN_BTN_MSG_NEXT
#else
#define PIN_KEY_MSGP -1
#endif
#endif
#ifndef PIN_KEY_PGEP
#ifdef PIN_BTN_PAGE
#define PIN_KEY_PGEP PIN_BTN_PAGE
#else
#define PIN_KEY_PGEP -1
#endif
#endif
#ifndef PIN_KEY_ATCMSG
#ifdef PIN_BTN_ATC_MSG
#define PIN_KEY_ATCMSG PIN_BTN_ATC_MSG
#else
#define PIN_KEY_ATCMSG -1
#endif
#endif
#ifndef PIN_ATC_MSG_LED
#define PIN_ATC_MSG_LED -1
#endif
#ifndef ATC_MSG_LED_ON
#define ATC_MSG_LED_ON HIGH
#endif

// ---------------------------------------------------------------- display --
// Drawn into a PSRAM frame buffer and pushed in one go, so updates don't flicker
// (NO_CANVAS in the board file: straight to the panel).
#ifdef NO_CANVAS
Arduino_GFX *gfx = boardPanel();
#else
Arduino_GFX *gfx = new Arduino_Canvas(480, 272, boardPanel(), CANVAS_X, CANVAS_Y);
#endif
const char *const boardName = BOARD_NAME;

static void backlight(int pct) {
  int v = map(pct, 0, 100, 0, 255);
  analogWrite(LCD_BL, LCD_BL_ON == HIGH ? v : 255 - v);
}

static const uint16_t C_BLACK = 0x0000;
static const uint16_t C_WHITE = 0xFFFF;
static const uint16_t C_GREEN = 0x07E0;
static const uint16_t C_CYAN  = 0x07FF;
static const uint16_t C_AMBER = 0xFDE0;   // 255,191,0
static const uint16_t C_DIM   = 0x7BEF;

// Layout - identical to the web page (baselines in pixels)
static const int W = 480, H = 272;
static const int ADV_BIG = 14, ADV_SMALL = 11;          // FreeMonoBold 12pt / 9pt advances
static const int Y_HDR = 22, X0 = 30;
static const int Y_LINE[5] = {58, 82, 106, 130, 154};
static const int Y_SEP = 172, VX1 = 125, VX2 = 355, V_TOP = 180;
static const int Y_ROW[2] = {205, 251};
static const int Y_SYS = 200, Y_LBL = 229, Y_VAL = 252, X_MSG = 185, X_PG = 295;

// Dark screen: the bridge says the DCDU has no power (sim and vPilot closed), or the
// bridge has been gone this long. A key lights it for WAKE_MS to show why.
static const uint32_t LINK_SLEEP_MS = 60000;
static const uint32_t WAKE_MS = 30000;

// ------------------------------------------------------------------- state --
static JsonDocument screenDoc;        // last screen from the bridge
static bool haveScreen = false;
bool dirty = true;
static bool lampOn = false;
static int brightness = 80;           // percent, from the bridge (BRT/DIM keys)
static bool dark = false;             // screen and backlight off
static uint32_t lastLinked = 0;       // last time the bridge was there (0 = not since start)
static uint32_t wakeUntil = 0;        // a key lit the screen until then
static bool awake = false;

// ------------------------------------------------------------------ helpers --
static uint16_t colour(const char *c) {
  if (!c || !*c) return C_WHITE;
  switch (c[0]) {
    case 'g': return C_GREEN;
    case 'c': return C_CYAN;
    case 'a': return C_AMBER;
    case 'k': return C_BLACK;
    default:  return C_WHITE;
  }
}

static void useBig()   { gfx->setFont(&FreeMonoBold12pt7b); }
static void useSmall() { gfx->setFont(&FreeMonoBold9pt7b); }

enum Align { LEFT, RIGHT, CENTER };

// Draw text with its baseline at y. Returns the width in pixels.
static int text(const char *s, int x, int y, uint16_t c, bool big, Align a) {
  if (!s || !*s) return 0;
  int adv = big ? ADV_BIG : ADV_SMALL;
  int w = strlen(s) * adv;
  if (a == RIGHT) x -= w;
  else if (a == CENTER) x -= w / 2;
  big ? useBig() : useSmall();
  gfx->setTextColor(c);
  gfx->setCursor(x, y);
  gfx->print(s);
  return w;
}

// ------------------------------------------------------------------ drawing --
static void drawWaiting() {
  // Like the real unit at power-up, plus how we're trying to reach the bridge.
  text("WAITING FOR DATA", W / 2, 110, C_GREEN, true, CENTER);
  char a[48] = "", b[48] = "", c[48];
  switch (wifiMode() ? wifiState() : WS_OFF) {
    case WS_OFF:
      strcpy(a, "CONNECT USB TO THE SIM PC");
      strcpy(b, "AND START THE DCDU BRIDGE");
      break;
    case WS_JOINING:
      snprintf(a, sizeof a, "WIFI %s", cfg.ssid);
      strcpy(b, "CONNECTING");
      break;
    case WS_FAILED:
      snprintf(a, sizeof a, "WIFI %s FAILED", cfg.ssid);
      strcpy(b, "CONNECT USB TO SET UP AGAIN");
      break;
    case WS_UP:
      if (*bridgeTarget()) snprintf(a, sizeof a, "SIM PC %s:%u", bridgeTarget(), cfg.port);
      else strcpy(a, "SEARCHING FOR SIM PC");
      snprintf(b, sizeof b, "THIS UNIT %s", WiFi.localIP().toString().c_str());
      break;
  }
  snprintf(c, sizeof c, "%s  FW %s", deviceId, FW_VERSION);
  text(a, W / 2, 160, C_DIM, false, CENTER);
  text(b, W / 2, 185, C_DIM, false, CENTER);
  text(c, W / 2, 240, C_DIM, false, CENTER);
}

static bool linked() { return usbUp() || wsUp(); }
static bool bridgeGone() { return !linked() && millis() - lastLinked > LINK_SLEEP_MS; }
static bool screenOff() { return haveScreen && !strcmp(screenDoc["pwr"] | "on", "off"); }

static void drawLines(JsonObjectConst s) {
  // message lines: each line is [[text, colour], ...]
  int i = 0;
  for (JsonArrayConst line : s["ln"].as<JsonArrayConst>()) {
    if (i >= 5) break;
    int x = X0;
    for (JsonArrayConst span : line) {
      x += text(span[0] | "", x, Y_LINE[i], colour(span[1] | "w"), true, LEFT);
    }
    i++;
  }
}

static void drawScreen() {
  JsonObjectConst s = screenDoc.as<JsonObjectConst>();
  const char *pwr = s["pwr"] | "on";
  if (!strcmp(pwr, "test") || !strcmp(pwr, "wait")) {   // powering up: the text only
    drawLines(s);
    return;
  }

  // header + status field
  text(s["hdr"] | "", 8, Y_HDR, C_GREEN, false, LEFT);
  JsonArrayConst st = s["st"].as<JsonArrayConst>();
  if (!st.isNull()) {
    const char *t = st[0] | "";
    const char *style = st[1] | "";
    if (style[0] == 'b') {                          // inverse box: "bc" cyan, "bg" green
      int w = strlen(t) * ADV_BIG + 8;
      gfx->fillRect(472 - w, Y_HDR + 2 - 19, w, 24, style[1] == 'c' ? C_CYAN : C_GREEN);
      text(t, 468, Y_HDR + 2, C_BLACK, true, RIGHT);
    } else if (style[0] == 'w') {
      text(t, 472, Y_HDR + 2, C_WHITE, false, RIGHT);
    } else {
      text(t, 472, Y_HDR + 2, colour(style), true, RIGHT);
    }
  }

  drawLines(s);

  // separators
  gfx->fillRect(8, Y_SEP, 464, 2, C_WHITE);
  gfx->fillRect(VX1, V_TOP, 2, H - V_TOP, C_WHITE);
  gfx->fillRect(VX2, V_TOP, 2, H - V_TOP, C_WHITE);

  // centre column: status line, MSG n / m, PG n / m
  if (!usbUp() && !wsUp()) {
    text("LINK LOST", W / 2, Y_SYS, C_AMBER, false, CENTER);
  } else {
    JsonArrayConst sys = s["sys"].as<JsonArrayConst>();
    if (!sys.isNull()) text(sys[0] | "", W / 2, Y_SYS, colour(sys[1] | "w"), false, CENTER);
  }
  char buf[16];
  const char *msg = s["msg"] | "";
  if (*msg) {
    text("MSG", X_MSG, Y_LBL, C_WHITE, false, CENTER);
    String v = String(msg); v.replace("/", " / ");
    text(v.c_str(), X_MSG, Y_VAL, C_WHITE, false, CENTER);
  }
  const char *pg = s["pg"] | "";
  if (*pg) {
    text("PG", X_PG, Y_LBL, C_WHITE, false, CENTER);
    String v = String(pg); v.replace("/", " / ");
    text(v.c_str(), X_PG, Y_VAL, C_WHITE, false, CENTER);
  }

  // prompts: "k": {"L1": [label, active, pressed], ...}
  for (JsonPairConst kv : s["k"].as<JsonObjectConst>()) {
    const char *key = kv.key().c_str();
    JsonArrayConst p = kv.value().as<JsonArrayConst>();
    bool left = key[0] == 'L';
    int y = Y_ROW[key[1] == '1' ? 0 : 1];
    bool active = p[1] | false, pressed = p[2] | false;
    snprintf(buf, sizeof buf, active ? (left ? "*%s" : "%s*") : "%s", (const char *)(p[0] | ""));
    int w = strlen(buf) * ADV_SMALL;
    if (pressed) gfx->fillRect(left ? 4 : 476 - w - 6, y - 16, w + 6, 21, C_CYAN);
    text(buf, left ? 7 : 473, y, pressed ? C_BLACK : C_CYAN, false, left ? LEFT : RIGHT);
  }
}


static void redraw() {
  gfx->fillScreen(C_BLACK);
  if (dark) {}
  else if (haveScreen && !bridgeGone() && (linked() || !screenOff())) drawScreen();
  else drawWaiting();                       // no bridge yet, or woken by a key
  gfx->flush();
  dirty = false;
}

static void applyBrightness() {
  backlight(dark ? 0 : brightness);
}

void wake() {
  wakeUntil = millis() + WAKE_MS;
  awake = true;
  dirty = true;
}

static void serviceDark() {
  if (linked()) lastLinked = millis();
  if (awake && (int32_t)(millis() - wakeUntil) >= 0) awake = false;
  // the bridge says off: dark, whatever the keys do. Bridge gone (or gone while the
  // DCDU was off): dark too, but a key shows the waiting page with the link details.
  bool d = awake ? (linked() && screenOff()) : (screenOff() || bridgeGone());
  static bool gone = false;
  if (bridgeGone() != gone) { gone = !gone; dirty = true; }  // stale screen -> waiting page
  if (d != dark) {
    dark = d;
    Serial.println(dark ? "screen off" : "screen on");
    applyBrightness();
    dirty = true;
  }
}

// ------------------------------------------------------------------ screen --
void takeScreen(JsonVariantConst s) {
  if (s.isNull()) return;
  screenDoc.set(s);
  haveScreen = true;
  lampOn = s["lamp"] | false;
  int br = s["br"] | brightness;
  if (br != brightness) { brightness = br; applyBrightness(); }
  dirty = true;
}

// ----------------------------------------------------------------- keys --
struct Key {
  int pin;
  const char *name;
  bool last;
  uint32_t changed;
};

static Key keys[] = {
  {PIN_KEY_L1, "L1"}, {PIN_KEY_L2, "L2"}, {PIN_KEY_R1, "R1"}, {PIN_KEY_R2, "R2"},
  {PIN_KEY_MSGM, "MSGM"}, {PIN_KEY_MSGP, "MSGP"}, {PIN_KEY_PGEM, "PGEM"}, {PIN_KEY_PGEP, "PGEP"},
  {PIN_KEY_BRT, "BRT"}, {PIN_KEY_DIM, "DIM"}, {PIN_KEY_PRINT, "PRINT"}, {PIN_KEY_ATCMSG, "ATCMSG"},
};

void fittedKeys(JsonArray out) {
  for (Key &k : keys)
    if (k.pin >= 0) out.add(k.name);
}

static void serviceKeys() {
  for (Key &k : keys) {
    if (k.pin < 0) continue;
    bool down = digitalRead(k.pin) == LOW;
    if (down != k.last && millis() - k.changed > 30) {     // 30 ms debounce
      k.last = down;
      k.changed = millis();
      if (down) sendKey(k.name);
    }
  }
}

static void serviceLamp() {
#if PIN_ATC_MSG_LED >= 0
  bool on = lampOn && !dark && linked() && ((millis() / 500) % 2 == 0);
  digitalWrite(PIN_ATC_MSG_LED, on ? ATC_MSG_LED_ON : !ATC_MSG_LED_ON);
#endif
}

// ------------------------------------------------------------------ setup --
void setup() {
  // USB serial carries the link to the bridge (dcdu_link.cpp): room for a whole screen
#if ARDUINO_USB_CDC_ON_BOOT && ARDUINO_USB_MODE      // "Hardware CDC and JTAG" (default)
  Serial.setTxBufferSize(4096);
  Serial.setTxTimeoutMs(20);          // a PC that isn't reading never stalls the display
#endif
  Serial.setRxBufferSize(8192);
  Serial.begin(115200);
  delay(200);
  Serial.println("\nDCDU display " FW_VERSION " starting");

  for (Key &k : keys)
    if (k.pin >= 0) pinMode(k.pin, INPUT_PULLUP);
#if PIN_ATC_MSG_LED >= 0
  pinMode(PIN_ATC_MSG_LED, OUTPUT);
#endif

  if (!gfx->begin()) {
    Serial.println("Display init failed - is PSRAM enabled in Tools (OPI PSRAM on the JC4827W543C)?");
  }
  pinMode(LCD_BL, OUTPUT);
  applyBrightness();
  gfx->setTextWrap(false);

  wifiSetup();
  linkSetup();
  redraw();
}

// ------------------------------------------------------------------- loop --
void loop() {
  linkLoop();
  wifiLoop();
  serviceKeys();
  serviceDark();
  serviceLamp();
  if (dirty) redraw();
}
