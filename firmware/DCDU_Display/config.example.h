// vDCDU display firmware. Copyright (c) 2026 SkySense Labs (https://www.skysenselabs.com)
// PolyForm Noncommercial License 1.0.0: see firmware/LICENSE.md.
#pragma once
// ---------------------------------------------------------------------------
// DCDU display: optional settings. You don't need this file: without it the display
// works over USB, and Wi-Fi is set up from the web page's Display tab, which saves it
// on the display. Copy this file to config.h to wire up physical keys, turn the screen
// over, use your own board (BOARD_CUSTOM), or pre-set Wi-Fi.
// ---------------------------------------------------------------------------

// Wi-Fi defaults (2.4 GHz only). Only used until Wi-Fi is changed from the web page;
// after that the display keeps what was set there. Leave empty for USB.
#define WIFI_SSID      ""
#define WIFI_PASSWORD  ""
#define DEVICE_NAME    "dcdu"

// The sim PC running dcdu_bridge.py. Leave empty: the display finds it by itself
// (the bridge sends its address, or it is found on the network with mDNS).
#define BRIDGE_HOST    ""
#define BRIDGE_PORT    8765

// 0 = normal, 2 = upside down (if the panel ends up mounted flipped)
#define SCREEN_ROTATION 0

// DCDU keys. Wire each key between a free GPIO and GND and put the GPIO number
// here; -1 = not fitted (use the keys on the web page instead). Brightness is
// set with BRT / DIM (on the web page or here) and shared with the web page.
#define PIN_KEY_L1      -1   // line key left 1   (e.g. *UNABLE, *CANCEL)
#define PIN_KEY_L2      -1   // line key left 2
#define PIN_KEY_R1      -1   // line key right 1  (e.g. STBY*)
#define PIN_KEY_R2      -1   // line key right 2  (e.g. WILCO*, SEND*, CLOSE*)
#define PIN_KEY_MSGM    -1   // MSG-
#define PIN_KEY_MSGP    -1   // MSG+
#define PIN_KEY_PGEM    -1   // PGE-
#define PIN_KEY_PGEP    -1   // PGE+
#define PIN_KEY_BRT     -1
#define PIN_KEY_DIM     -1
#define PIN_KEY_PRINT   -1
#define PIN_KEY_ATCMSG  -1   // ATC MSG pushbutton (stops the flashing)

// ATC MSG lamp (flashes for a new ATC message). -1 = not fitted.
#define PIN_ATC_MSG_LED -1
#define ATC_MSG_LED_ON  HIGH

// Another board than the Guition JC4827W543C: copy board_custom.example.h to
// board_custom.h, fill it in, and uncomment this (see docs/diy-display.md).
// #define BOARD_CUSTOM
