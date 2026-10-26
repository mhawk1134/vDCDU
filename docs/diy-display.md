# Build your own display

A hardware DCDU for your cockpit shows exactly the same screen as the web page, and its keys work the same. Plug it into the sim PC with a USB cable and it works with nothing to set up; it can also run on Wi-Fi. Everything else in vDCDU works without one.

> Install vDCDU on the PC first ([installation guide](install.md)) and check the web DCDU works.

- [The easy way: Guition JC4827W543C](#the-easy-way-guition-jc4827w543c)
- [Build and upload the firmware (Arduino IDE)](#build-and-upload-the-firmware-arduino-ide)
- [Connect it to vDCDU](#connect-it-to-vdcdu)
- [Keys and the ATC MSG light](#keys-and-the-atc-msg-light)
- [Other boards and screens](#other-boards-and-screens)
- [Network notes](#network-notes)

---

## The easy way: Guition JC4827W543C

The firmware is ready to build for this board:

| Item | Notes |
|---|---|
| **Guition JC4827W543C** | ESP32-S3 board (4 MB flash, 8 MB PSRAM) with a built-in 4.3" 480×272 screen. Sold on AliExpress and similar; get the version **without** a case if you'll build it into a panel. |
| USB-C cable | Must carry data, not just power |
| Optional: 12 push-buttons | For real DCDU keys. Without them, use the keys on the web page. |
| Optional: an LED | For the ATC MSG light |
| Optional: 2.4 GHz Wi-Fi | Only to run the display without a cable to the PC. The ESP32 can't use 5 GHz networks. |

## Build and upload the firmware (Arduino IDE)

You build the firmware yourself, so it fits your wiring: key pins, an upside-down screen, or another board. You get the source in this repository's [`firmware/`](../firmware/) folder: click the green **Code** button on the [main page](https://github.com/mhawk1134/vDCDU) → **Download ZIP**, and unzip it.

1. Install the **Arduino IDE 2** from [arduino.cc/en/software](https://www.arduino.cc/en/software).
2. **File → Preferences → Additional boards manager URLs:** add `https://espressif.github.io/arduino-esp32/package_esp32_index.json`. Then **Tools → Board → Boards Manager**: install **esp32 by Espressif Systems** (tested with 3.3.x).
3. **Tools → Manage Libraries…**: install **GFX Library for Arduino** (by Moon On Our Nation, tested with 1.6.9), **WebSockets** (by Markus Sattler, 2.7.2) and **ArduinoJson** (by Benoit Blanchon, 7.4.3).
4. Open `firmware\DCDU_Display\DCDU_Display.ino`. The code is in the `dcdu…` tabs; that file itself is empty on purpose.
5. Optional: copy `config.example.h` to **`config.h`** in the same folder and edit it (keys, rotation, see below). It appears as a tab in the IDE.
6. On the vDCDU page, turn off **Display → Look for a display on USB**, plug the board in, and under **Tools** set:

| Setting | JC4827W543C |
|---|---|
| Board | **ESP32S3 Dev Module** |
| PSRAM | **OPI PSRAM** (required) |
| Flash Size | **4MB (32Mb)** |
| USB CDC On Boot | **Enabled** (vDCDU talks to the display through it) |
| Partition Scheme | **Huge APP (3MB No OTA/1MB SPIFFS)** |
| Port | the COM port that appeared when you plugged it in |

7. Click **Upload** (the → arrow), then turn **Look for a display on USB** back on.

**Upload fails?** Try another cable (many only charge), and close any Serial Monitor. If it still fails: hold the board's **BOOT** button, press and release **RESET**, release **BOOT**, and upload again. In that mode the board can always be written.

**Updating later:** download the new version, copy your `config.h` (and `board_custom.h`, if you have one) into its `DCDU_Display` folder, and upload it the same way. The display keeps its Wi-Fi settings.

**You should see:** `WAITING FOR DATA`, then the DCDU once vDCDU finds it.

---

## Connect it to vDCDU

### Over USB (simplest)

Leave the display plugged into the sim PC. Nothing to set up: within about 5 seconds it shows the same DCDU as the web page, and the **Display** tab shows its card with a green **USB**.

A board whose USB goes through a **USB-serial chip** (CH340, CP210x, FTDI: most boards that aren't ESP32-S3) isn't searched by itself, because other cockpit hardware uses the same chips. On the Display tab open **My display isn't listed** and tick its COM port once.

### Over Wi-Fi (optional)

Set it up once with the display plugged into the PC by USB:

1. On the vDCDU page, go to the **Display** tab.
2. Under **Connection**, click **Wi-Fi**. The display looks for networks; pick yours (2.4 GHz) or choose **Other network…** to type its name.
3. Type the Wi-Fi password and click **Connect**.

**You should see:** after up to 15 seconds, *Connected to <network>, display address 192.168.…*. vDCDU also tells the display this PC's address. Now unplug the display and power it from any USB power supply: after a few seconds it shows the DCDU again, over Wi-Fi, with a green **Wi-Fi** on the Display tab.

**"Wrong password" or "network not found":** the display stays on USB and keeps its old settings. Check the password, and that the network is 2.4 GHz (many routers have separate 2.4 and 5 GHz names).

- **Back to USB:** plug it into the PC and click **USB cable** on the Display tab. Wi-Fi is turned off but remembered (**Use it** switches back). **Forget Wi-Fi** erases it.
- The settings are kept on the display, through power-off and firmware updates. The password goes from the web page to the display over USB and is stored only on the display; vDCDU never saves or logs it.
- **Wi-Fi from a browser instead (Improv):** with vDCDU quit and the display plugged in, open [improv-wifi.com](https://www.improv-wifi.com) in Chrome or Edge, click **Connect device to Wi-Fi**, choose the display's port and enter your Wi-Fi.

### What the waiting screen tells you

Under `WAITING FOR DATA` the display shows how it's trying to reach vDCDU:

| Lines | Meaning |
|---|---|
| `CONNECT USB TO THE SIM PC` | USB mode, and vDCDU isn't talking to it yet. Is vDCDU running, with **Look for a display on USB** on (and the port ticked, for a USB-serial board)? |
| `WIFI <name>` / `CONNECTING` | Joining your Wi-Fi |
| `WIFI <name> FAILED` | Couldn't join for 30 seconds (router off, password changed…). Plug it into the PC and set Wi-Fi again. |
| `SEARCHING FOR SIM PC` / `THIS UNIT <address>` | On Wi-Fi, looking for vDCDU on the network |
| `SIM PC <address>:8765` / `THIS UNIT <address>` | On Wi-Fi, trying vDCDU at that address |

If it loses vDCDU after showing the DCDU, the status line shows `LINK LOST` until it reconnects by itself. After a minute without vDCDU (also at start-up) the screen and backlight go off; any key lights them for 30 seconds with this waiting screen. While vDCDU runs, the screen is also dark when the sim and vPilot are both closed, like the real DCDU without aircraft power. The Arduino IDE's **Serial Monitor** (115200 baud, with **Look for a display on USB** off) shows more detail.

---

## Keys and the ATC MSG light

Wire each push-button between a free GPIO pin and **GND** (no resistor needed: the firmware turns on the pin's pull-up). Then put the GPIO numbers in `config.h` and upload the firmware again ([how](#build-and-upload-the-firmware-arduino-ide)). Use `-1` for keys you don't fit; those stay usable on the web page.

| Setting | Key |
|---|---|
| `PIN_KEY_L1`, `PIN_KEY_L2` | Left line keys (top, bottom) |
| `PIN_KEY_R1`, `PIN_KEY_R2` | Right line keys (top, bottom) |
| `PIN_KEY_MSGM`, `PIN_KEY_MSGP` | MSG− / MSG+ |
| `PIN_KEY_PGEM`, `PIN_KEY_PGEP` | PGE− / PGE+ |
| `PIN_KEY_BRT`, `PIN_KEY_DIM` | BRT / DIM |
| `PIN_KEY_PRINT` | PRINT |
| `PIN_KEY_ATCMSG` | ATC MSG pushbutton |
| `PIN_ATC_MSG_LED` | ATC MSG light: an LED with a suitable resistor (e.g. 330 Ω), or a transistor for a bigger lamp. `ATC_MSG_LED_ON` sets whether the pin goes `HIGH` or `LOW` to light it. |

`SCREEN_ROTATION 2` in `config.h` turns the screen over, if the unit is mounted upside down.

**Free pins on the JC4827W543C:** check the board's pinout. The screen, backlight (GPIO 1), touch controller (GPIO 3, 4, 8, 38) and USB (GPIO 19, 20) are taken. GPIO 0 is the BOOT button.

**Test your wiring:** on the **Display** tab, the display's card shows the 12 keys. Keys not set in `config.h` are greyed out. Press a key on the display and it lights up on the card.

---

## Other boards and screens

The firmware is written so it can be moved to other ESP32 boards and screens. You need some Arduino experience for this.

### What works

| | |
|---|---|
| Chip | **ESP32-S3** (best: its own USB port is found by vDCDU by itself) or a plain **ESP32** (USB-serial chip: tick its port on the Display tab). Both have the Wi-Fi the firmware uses. |
| Screen | Any screen [GFX Library for Arduino](https://github.com/moononournation/Arduino_GFX) supports (SPI, QSPI, parallel, RGB), **at least 480×272 in landscape**. A bigger screen shows the DCDU centred. |
| Memory | **PSRAM** (2 MB or more) for a flicker-free screen. Without PSRAM, set `NO_CANVAS` (see below): it works, with some flicker when the screen changes. |
| Flash | 4 MB or more |

**Smaller screens** (for example the 2.8" and 3.5" 320×240 / 480×320 "Cheap Yellow Display" boards): 480×320 works. Below 480×272, the DCDU's 30-column text doesn't fit at its font size, and the firmware stops with an error when you compile. A small-screen layout would need its own fonts and the layout numbers in `dcdu.cpp` (and, to match, in the web page) changed. If you make one, please share it.

### Steps

1. Find the lines that set up your screen. The board seller's demo code, or **File → Examples → GFX Library for Arduino** (look for your board or driver chip), has two lines like these:
   ```cpp
   Arduino_DataBus *bus = new Arduino_ESP32SPI(...);
   Arduino_GFX *gfx = new Arduino_ILI9488_18bit(bus, ...);
   ```
2. In the `firmware/DCDU_Display` folder, copy **`board_custom.example.h`** to **`board_custom.h`** and fill it in:
   - `BOARD_NAME`: any name; it's shown on the Display tab.
   - `PANEL_W`, `PANEL_H`: the screen size in landscape.
   - `LCD_BL`: the backlight pin (the firmware dims it with PWM) and `LCD_BL_ON` (`LOW` if the backlight is on when the pin is low).
   - `boardPanel()`: your two lines from step 1, returning the screen. Use `SCREEN_ROTATION` (from `config.h`) for the rotation, plus 1 if the panel is portrait by default.
   - `NO_CANVAS`: uncomment on a board without PSRAM.
3. Copy `config.example.h` to **`config.h`** and uncomment **`#define BOARD_CUSTOM`**. Add your key pins.
4. Set the **Tools** menu for your board: for example **ESP32S3 Dev Module** with the right PSRAM type (OPI or QSPI: see the board's description) and flash size, or **ESP32 Dev Module** for a plain ESP32. Pick a partition scheme with at least 1.3 MB for the app, such as **Huge APP**. On an ESP32-S3 using its own USB port, set **USB CDC On Boot: Enabled**.
5. Upload, and check the Display tab: your board's name, then the keys.

**Nothing on screen?** Open the Serial Monitor (115200 baud, with **Look for a display on USB** off) and check the start-up messages. Wrong colours mean a different colour order or an IPS setting in `boardPanel()`; a mirrored or rotated picture means the rotation. The screen works but vDCDU doesn't find it: see [What the waiting screen tells you](#what-the-waiting-screen-tells-you).

### Something else entirely

The display only draws what vDCDU sends it and sends key presses back: all the DCDU logic is in vDCDU on the PC. Any device that speaks the [display protocol](display-protocol.md) (JSON lines over USB serial, or a WebSocket over Wi-Fi) can be a DCDU display.

### Share your build

Ports to other boards are very welcome: share your `board_custom.h` and the Tools settings in [Discussions](https://github.com/mhawk1134/vDCDU/discussions). The firmware's [licence](../firmware/LICENSE.md) allows this for non-commercial use.

---

## Network notes

Only for a display on Wi-Fi:

- The display connects to vDCDU on **TCP port 8765**. If your Wi-Fi puts smart devices on a separate "IoT" or guest network, allow that network to reach the sim PC on port 8765, or put the display on the same network as the PC.
- **How it finds the PC:** when you set up Wi-Fi on the Display tab, vDCDU gives the display this PC's address. If the PC's address changes later, the display looks for vDCDU on the network (mDNS, `_dcdu._tcp`). mDNS only works within one network, unless your router passes it between networks. Give the sim PC a fixed address (a DHCP reservation in your router) and this never matters; or type the PC's address on the Display tab.
- Windows Firewall must allow vDCDU on **Private** networks, and Windows must call your network Private. The setup program offers both; see [Troubleshooting](troubleshooting.md#tablet-cant-open-the-page).
