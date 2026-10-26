# Build your own display

A hardware DCDU shows the same screen as the web page, and its keys work the same. It connects to vDCDU by USB or Wi-Fi.

## Parts

| Part | Notes |
|---|---|
| **Guition JC4827W543C** | ESP32-S3 board with a 4.3" 480×272 screen (AliExpress). The firmware is ready for it. Other boards: see [below](#other-boards). |
| USB-C data cable | Not a charge-only cable |
| Optional: 12 push-buttons, 1 LED | DCDU keys and the ATC MSG light |

## Build and upload the firmware

1. Install the [Arduino IDE 2](https://www.arduino.cc/en/software).
2. **File → Preferences → Additional boards manager URLs:** add
   `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
   Then **Tools → Board → Boards Manager:** install **esp32** by Espressif.
3. **Tools → Manage Libraries:** install **GFX Library for Arduino**, **WebSockets** (Markus Sattler) and **ArduinoJson**.
4. Download this repository (**Code → Download ZIP**) and open `firmware/DCDU_Display/DCDU_Display.ino`.
5. Copy `config.example.h` to `config.h` and set your [key pins](#keys).
6. On the vDCDU page, **Display** tab: turn off **Look for a display on USB**.
7. **Tools** settings:

| Setting | Value |
|---|---|
| Board | ESP32S3 Dev Module |
| PSRAM | OPI PSRAM |
| Flash Size | 4MB |
| USB CDC On Boot | Enabled |
| Partition Scheme | Huge APP (3MB No OTA) |

8. Click **Upload**, then turn **Look for a display on USB** back on.

The display shows `WAITING FOR DATA`, then the DCDU.

**Upload fails?** Hold **BOOT**, press **RESET**, release **BOOT**, and upload again.

**Updating:** copy your `config.h` into the new version's folder and upload again. Wi-Fi settings are kept.

## Connect

**USB:** leave it plugged into the PC. Nothing to set up.

**Wi-Fi** (2.4 GHz), set up once over USB:
1. **Display** tab → **Wi-Fi** → pick your network → password → **Connect**.
2. Unplug it and power it from any USB charger. It finds the PC by itself.

**Back to USB:** plug it in and click **USB cable**.

## Keys

Wire each button between a free GPIO and **GND**, and set its pin in `config.h` (`-1` = not fitted):

| Setting | Key |
|---|---|
| `PIN_KEY_L1`, `PIN_KEY_L2`, `PIN_KEY_R1`, `PIN_KEY_R2` | Line keys |
| `PIN_KEY_MSGM`, `PIN_KEY_MSGP` | MSG− / MSG+ |
| `PIN_KEY_PGEM`, `PIN_KEY_PGEP` | PGE− / PGE+ |
| `PIN_KEY_BRT`, `PIN_KEY_DIM`, `PIN_KEY_PRINT` | BRT, DIM, PRINT |
| `PIN_KEY_ATCMSG` | ATC MSG button |
| `PIN_ATC_MSG_LED` | ATC MSG light (LED + resistor) |

`SCREEN_ROTATION 2` turns the screen upside down.

On the JC4827W543C, GPIO 1, 3, 4, 8, 19, 20 and 38 are taken (screen, touch, USB).

**Test your wiring:** the **Display** tab shows the 12 keys. Press one on the display and it lights up.

## If it doesn't connect

The waiting screen tells you why:

| Shows | Do |
|---|---|
| `CONNECT USB TO THE SIM PC` | Start vDCDU. Check **Look for a display on USB** is on. |
| `WIFI <name> FAILED` | Plug it in by USB and set Wi-Fi again |
| `SEARCHING FOR SIM PC` | Windows must call your network **Private**, and the firewall must allow vDCDU (run setup again). A separate IoT network must be allowed to reach the PC on port 8765. |

A board with a USB-serial chip (CH340, CP210x) must be ticked once under **Display → My display isn't listed**.

## Other boards

The firmware works on other ESP32 / ESP32-S3 boards with a screen of **at least 480×272** that [GFX Library for Arduino](https://github.com/moononournation/Arduino_GFX) supports.

1. Copy `board_custom.example.h` to `board_custom.h`.
2. Fill in your screen's setup lines (from your board's demo code or the library's examples), size and backlight pin.
3. In `config.h`, uncomment `#define BOARD_CUSTOM`.
4. Set the **Tools** menu for your board and upload.

No PSRAM? Add `#define NO_CANVAS` to `board_custom.h`. Smaller screens (e.g. 320×240) aren't supported yet.

Share your build in [Discussions](https://github.com/mhawk1134/vDCDU/discussions).

Building something else entirely? See the [display protocol](display-protocol.md).
