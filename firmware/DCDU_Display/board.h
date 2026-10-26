// vDCDU display firmware. Copyright (c) 2026 mhawk1134 (https://github.com/mhawk1134/vDCDU)
// PolyForm Noncommercial License 1.0.0: see firmware/LICENSE.md.
#pragma once
// Which board this firmware is built for. Included by dcdu.cpp only.
//
// The ready-made one is the Guition JC4827W543C (board_jc4827w543c.h). For another board:
// copy board_custom.example.h to board_custom.h, fill it in, and put
//   #define BOARD_CUSTOM
// in config.h. See docs/diy-display.md.

#include <Arduino_GFX_Library.h>

#if defined(BOARD_CUSTOM)
#include "board_custom.h"
#else
#include "board_jc4827w543c.h"
#endif

// The DCDU screen is drawn at 480x272 (the same layout as the web page). A bigger panel
// shows it centred; move it with CANVAS_X / CANVAS_Y in the board file.
#if PANEL_W < 480 || PANEL_H < 272
#error "The DCDU layout needs a panel of at least 480x272 (see docs/diy-display.md, Smaller screens)"
#endif
#ifndef CANVAS_X
#define CANVAS_X ((PANEL_W - 480) / 2)
#endif
#ifndef CANVAS_Y
#define CANVAS_Y ((PANEL_H - 272) / 2)
#endif
#ifndef LCD_BL_ON
#define LCD_BL_ON HIGH                   // backlight pin level for "on" (LOW: inverted)
#endif
