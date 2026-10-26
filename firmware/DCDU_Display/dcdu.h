// vDCDU display firmware. Copyright (c) 2026 mhawk1134 (https://github.com/mhawk1134/vDCDU)
// PolyForm Noncommercial License 1.0.0: see firmware/LICENSE.md.
#pragma once
// Shared between dcdu.cpp (screen, keys), dcdu_link.cpp (USB + WebSocket to the
// bridge), dcdu_wifi.cpp (saved settings, Wi-Fi, Improv). The board itself (screen, backlight) is in board.h.

#include <Arduino.h>
#include <ArduinoJson.h>

#define FW_VERSION "0.3.0-rc11"

// config.h is optional: without it the display starts in USB mode and Wi-Fi is set
// up from the DCDU web page. Its values are only first-start defaults.
#if __has_include("config.h")
#include "config.h"
#endif

#ifndef DEVICE_NAME
#define DEVICE_NAME "dcdu"
#endif
#ifndef WIFI_SSID
#define WIFI_SSID ""
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD ""
#endif
#ifndef BRIDGE_HOST
#define BRIDGE_HOST ""
#endif
#ifndef BRIDGE_PORT
#define BRIDGE_PORT 8765
#endif
#ifndef SCREEN_ROTATION
#define SCREEN_ROTATION 0
#endif

// ----------------------------------------------------------- dcdu.cpp --
extern bool dirty;                       // redraw on the next loop
void takeScreen(JsonVariantConst s);     // a screen dict from the bridge
void wake();                             // a key was pressed: light a dark screen for a while
extern const char *const boardName;      // BOARD_NAME from the board file
void fittedKeys(JsonArray out);          // names of the keys wired up (config.h)

// ------------------------------------------------------ dcdu_link.cpp --
enum From { FROM_USB, FROM_WS, FROM_IMPROV };

void linkSetup();
void linkLoop();
bool usbUp();                            // the bridge is talking to us over USB
bool wsUp();                             // ... or over Wi-Fi
const char *bridgeTarget();              // Wi-Fi: host we're trying, "" while searching
void sendKey(const char *key);
void sendTo(From to, JsonDocument &doc); // reply on the link a command came from
void linkHostChanged();                  // saved host changed: reconnect

// ------------------------------------------------------ dcdu_wifi.cpp --
struct Settings {
  char mode[8];                          // "usb" or "wifi"
  char ssid[33];
  char pass[65];
  char host[64];                         // sim PC address ("" = find it with mDNS)
  uint16_t port;
};
extern Settings cfg;
extern char deviceId[16];                // "dcdu-1a2b3c", from the MAC address

enum WifiState { WS_OFF, WS_JOINING, WS_UP, WS_FAILED };

void wifiSetup();
void wifiLoop();
WifiState wifiState();
bool wifiMode();                         // cfg.mode is "wifi"
void wifiJoin(const char *ssid, const char *pass, From from);
void wifiScan(From from);
void wifiSetMode(const char *mode);
void wifiSetHost(const char *host, uint16_t port);
void wifiForget();
bool improvByte(uint8_t c, bool lineStart);   // true = byte belongs to an Improv frame

