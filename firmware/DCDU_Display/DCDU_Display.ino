// vDCDU display firmware. Copyright (c) 2026 mhawk1134 (https://github.com/mhawk1134/vDCDU)
// PolyForm Noncommercial License 1.0.0: see firmware/LICENSE.md.
// DCDU display for vDCDU. Ready-made for the Guition JC4827W543C (ESP32-S3, 4.3"
// 480x272); other boards: board.h.
//
// All the code is in the .cpp tabs; this file is intentionally empty (Arduino needs a
// .ino named after the folder).
//   dcdu.cpp       screen and keys
//   dcdu_link.cpp  link to the bridge: USB cable or Wi-Fi
//   dcdu_wifi.cpp  saved settings, Wi-Fi, Improv
//   dcdu.h         shared declarations
//   board.h        which board: board_jc4827w543c.h, or your own board_custom.h
//                  (copy of board_custom.example.h; see docs/diy-display.md)
//   config.h       optional: your own defaults and key pins (copy of config.example.h)
//
// No settings are needed: plug the display into the sim PC by USB and start vDCDU.
// Wi-Fi is set up on the vDCDU page's Display tab.
//
// Tools menu: Board "ESP32S3 Dev Module", PSRAM "OPI PSRAM", Flash Size "4MB (32Mb)",
//             USB CDC On Boot "Enabled", Partition Scheme "Huge APP (3MB No OTA/1MB
//             SPIFFS)".
// Libraries:  GFX Library for Arduino (Moon On Our Nation), WebSockets (Markus Sattler),
//             ArduinoJson (Benoit Blanchon).
