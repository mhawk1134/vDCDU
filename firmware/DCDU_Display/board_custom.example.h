// vDCDU display firmware. Copyright (c) 2026 SkySense Labs (https://www.skysenselabs.com)
// PolyForm Noncommercial License 1.0.0: see firmware/LICENSE.md.
#pragma once
// ---------------------------------------------------------------------------
// Your own board. Copy this file to board_custom.h, fill it in, and put
//   #define BOARD_CUSTOM
// in config.h. Full guide: docs/diy-display.md.
//
// You need to know your screen's driver chip, its resolution and how it's wired. The
// board seller's demo code, or the examples that come with "GFX Library for Arduino"
// (File > Examples > GFX Library for Arduino), usually have the exact lines: look for
// the "Arduino_DataBus *bus = ..." and "Arduino_GFX *gfx = ..." lines and copy them into
// boardPanel() below.
//
// The example below is a 3.5" 480x320 ILI9488 SPI screen wired to an ESP32-S3.
// The pin numbers are examples: use your own wiring.
// ---------------------------------------------------------------------------

#define BOARD_NAME "My DCDU display"      // shown on the web page's Display tab

// Panel size as the DCDU sees it (landscape). At least 480x272: the DCDU is drawn at
// 480x272 and a bigger panel shows it centred (move it with CANVAS_X / CANVAS_Y).
#define PANEL_W 480
#define PANEL_H 320

// Backlight: the GPIO that switches it (dimmed with PWM), and the level that means "on".
// No backlight pin? Use a free GPIO that isn't connected to anything.
#define LCD_BL   45
#define LCD_BL_ON HIGH

// The screen. SCREEN_ROTATION comes from config.h (0 normal, 2 upside down); add 1 to
// it here if your panel is portrait by default and must be turned to landscape.
static Arduino_GFX *boardPanel() {
  Arduino_DataBus *bus = new Arduino_ESP32SPI(9 /* DC */, 10 /* CS */, 12 /* SCK */,
                                              11 /* MOSI */, 13 /* MISO */);
  return new Arduino_ILI9488_18bit(bus, 14 /* RST */, (SCREEN_ROTATION + 1) % 4, false /* IPS */);
}

// The DCDU draws into a 480x272 frame buffer (about 260 KB) and sends it to the screen in
// one go, so nothing flickers. That needs PSRAM. A board without PSRAM (most plain ESP32
// boards) can draw straight to the screen instead, with some flicker on each update:
// #define NO_CANVAS
