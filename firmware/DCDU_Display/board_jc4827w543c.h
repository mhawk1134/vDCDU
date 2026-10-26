// vDCDU display firmware. Copyright (c) 2026 SkySense Labs (https://www.skysenselabs.com)
// PolyForm Noncommercial License 1.0.0: see firmware/LICENSE.md.
#pragma once
// Guition JC4827W543C: ESP32-S3 (WROOM-1-N4R8: 4 MB flash, 8 MB PSRAM), 4.3" 480x272 IPS,
// NV3041A on QSPI. Tools menu settings: see DCDU_Display.ino.
// Touch (GT911, not used yet): SDA 8, SCL 4, INT 3, RST 38.

#define BOARD_NAME "Guition JC4827W543C"
#define PANEL_W 480
#define PANEL_H 272

// The board's own wiring (old config.h files may still define these)
#ifndef LCD_CS
#define LCD_CS   45
#define LCD_SCK  47
#define LCD_D0   21
#define LCD_D1   48
#define LCD_D2   40
#define LCD_D3   39
#define LCD_BL    1
#endif

static Arduino_GFX *boardPanel() {
  Arduino_DataBus *bus = new Arduino_ESP32QSPI(LCD_CS, LCD_SCK, LCD_D0, LCD_D1, LCD_D2, LCD_D3);
  return new Arduino_NV3041A(bus, GFX_NOT_DEFINED, SCREEN_ROTATION, true /* IPS */);
}
