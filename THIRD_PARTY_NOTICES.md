# Third-party notices

vDCDU uses the following. Each keeps its own licence; none of them is covered by vDCDU's licences.

## In the vDCDU program (the setup program)

The setup program installs these with the program. Their licence texts are installed in `_internal\licenses` next to `DCDU.exe`, and listed in `THIRD_PARTY_NOTICES.txt`.

| Component | Used for | Licence |
|---|---|---|
| [Python 3](https://www.python.org) | Runs the program | Python Software Foundation License. Its Windows build includes OpenSSL (Apache 2.0), Expat (MIT), libffi (MIT), bzip2, zstd (BSD) and xz (public domain / 0BSD). |
| [websockets](https://github.com/python-websockets/websockets) | Web page and display connections | BSD 3-Clause |
| [pyserial](https://github.com/pyserial/pyserial) | Display on a USB cable | BSD 3-Clause |
| [python-zeroconf](https://github.com/python-zeroconf/python-zeroconf) | Lets a Wi-Fi display find the PC | LGPL 2.1 or later. Installed as separate, unmodified files in `_internal\zeroconf`, so you can replace it with your own build of it. |
| [ifaddr](https://github.com/ifaddr/ifaddr) | Used by python-zeroconf | MIT |
| [PyInstaller](https://pyinstaller.org) bootloader | Starts the program | GPL 2.0 or later, with an exception for programs under any licence |

## In the display firmware

The firmware source in `firmware/` doesn't include these libraries; the Arduino IDE installs them (see the [DIY display guide](docs/diy-display.md)). The ready-made firmware files on the releases page are built from this source with them. You can rebuild that firmware yourself from the source here, with the same or changed versions of these libraries.

| Component | Licence |
|---|---|
| [Arduino core for the ESP32](https://github.com/espressif/arduino-esp32) and [ESP-IDF](https://github.com/espressif/esp-idf) | LGPL 2.1 (Arduino core), Apache 2.0 (ESP-IDF) |
| [GFX Library for Arduino](https://github.com/moononournation/Arduino_GFX) | BSD |
| [WebSockets](https://github.com/Links2004/arduinoWebSockets) (Markus Sattler) | LGPL 2.1 |
| [ArduinoJson](https://arduinojson.org) | MIT |

Included in `firmware/`:

| Files | From | Licence |
|---|---|---|
| `FreeMonoBold9pt7b.h`, `FreeMonoBold12pt7b.h` | GNU FreeFont (FreeMono Bold), converted to the Adafruit GFX font format | GNU GPL 3 with the font exception. See [GNU FreeFont](https://www.gnu.org/software/freefont/). |

## Online services (used at run time, nothing included)

| Service | Used for |
|---|---|
| [Hoppie's ACARS](https://www.hoppie.nl/acars/) | CPDLC messages and replies. Needs your own logon code. |
| [VATSIM data feeds](https://data.vatsim.net) | Controllers, ATIS, flight plans, transceiver positions |
| [VATSpy data project](https://github.com/vatsimnetwork/vatspy-data-project) | Country, airport and FIR names, cached on your PC |

## vPilot plugin

The setup program compiles the small vPilot plugin on your PC with the C# compiler that comes with Windows, against the plugin interface in your own vPilot folder. vPilot isn't included.

## Behaviour reference

The DCDU behaviour follows the real A320 as described in [docs/dcdu-reference.md](docs/dcdu-reference.md), with the open-source [FlyByWire A32NX](https://github.com/flybywiresim/aircraft) DCDU as the main reference. No FlyByWire code is included.
