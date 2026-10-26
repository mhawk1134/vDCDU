# Installation

## 1. Download

Get **`vDCDU-Setup-<version>.exe`** from the [latest release](https://github.com/mhawk1134/vDCDU/releases/latest).

## 2. Install

1. **Close vPilot.**
2. Run the setup program. If Windows says *"Windows protected your PC"*, click **More info → Run anyway**.
3. Follow the pages:

| Page | What to do |
|---|---|
| vPilot | Check the folder it found (the one with `vPilot.exe`). |
| Hoppie ACARS (optional) | Only for CPDLC. Type your logon code, or leave it empty. |
| Tasks | **Start vDCDU with Microsoft Flight Simulator** (recommended).<br>**Let tablets and a Wi-Fi display connect**, if you'll use a tablet or Wi-Fi display. |

If Windows has your network set to **Public**, say **Yes** to make it **Private**. Otherwise tablets can't connect.

## 3. Open it

The DCDU opens in your browser at **http://localhost:8765**. Click once on the page so the message chime can play.

The DCDU stays dark until the sim is running or vPilot is connected, like the real one without aircraft power.

## 4. Check it

Connect vPilot to VATSIM. The DCDU powers up, and the indicators under it should show:

- **vPilot:** `connected <your callsign>`
- **Hoppie:** `ok <your callsign>`, or `off` if you skipped it

## On a tablet

1. On the PC, open PowerShell and type `ipconfig`. Note the **IPv4 Address** (e.g. `192.168.1.50`).
2. On the tablet, go to `http://192.168.1.50:8765` (use your address).

## Update

Run the new setup program. Your settings are kept.

## Uninstall

**Settings → Apps → vDCDU → Uninstall.** This also removes the vPilot plugin and the sim auto-start.
