# Troubleshooting

Start with the three status indicators under the DCDU on the web page. They tell you which part isn't working.

- [The status indicators](#the-status-indicators)
- [Messages on the DCDU's status line](#messages-on-the-dcdus-status-line)
- [Common problems](#common-problems)
- [Log files](#log-files)
- [Reporting a problem](#reporting-a-problem)

---

## The status indicators

Green dot = working, grey = waiting or not needed yet, amber = needs attention.

### Hoppie (CPDLC messages from ATC)

| Status | Meaning | What to do |
|---|---|---|
| `starting` | vDCDU has just started | Wait a few seconds |
| `no logon code` | No Hoppie logon code saved | Run the setup program again and type it on the Hoppie page. |
| `waiting for callsign` | No callsign yet | Connect vPilot to VATSIM |
| `paused (vPilot not connected)` | You disconnected from VATSIM, so it stops checking | Normal after a flight |
| `ok <callsign>` | Checking Hoppie for messages to your callsign | Nothing, it's working |
| `error: …` | Hoppie refused the request, e.g. `illegal logon code` | Check your code: run the setup program again and type it once more. |
| `unreachable` | Couldn't reach Hoppie's server | Check the internet connection; it retries by itself |
| `demo (replies simulated)` | Running with `--demo` | Nothing; restart without `--demo` to fly for real |

### vPilot (callsign, PDCs, private messages)

| Status | Meaning | What to do |
|---|---|---|
| `not running` | vPilot isn't open (or was closed) | Start vPilot |
| `plugin not detected` | vPilot is open, but the plugin hasn't said anything | Close vPilot, run the setup program again (check the vPilot folder it shows), then start vPilot |
| `running, not connected` | vPilot is open with the plugin, not connected to VATSIM | Connect to VATSIM |
| `connected <callsign> (plugin 1.3)` | Connected; this callsign is used for Hoppie | Nothing, it's working |

### VATSIM data (ATIS and controller info)

| Status | Meaning | What to do |
|---|---|---|
| `not loaded yet` | Only downloaded when needed: auto ATIS once you have a callsign, or the ATC info tab | Nothing |
| `ok` | The last download worked | Nothing |
| `unreachable` | The last download failed | Check the internet connection; it retries by itself |

---

## The DCDU screen is dark

That's normal while Microsoft Flight Simulator isn't running and vPilot isn't connected: the DCDU has no "aircraft power". The **MSFS** and **vPilot** indicators on the web page show what vDCDU sees. The hardware display also goes dark a minute after it loses vDCDU, e.g. when the PC shuts down. Press any of its keys to light it for 30 seconds with the `WAITING FOR DATA` page, which shows how it's trying to reach the PC.

---

## Messages on the DCDU's status line

| Message | Meaning |
|---|---|
| `COM NOT INIT` | No Hoppie logon code, and vPilot isn't connected. Once vPilot connects this goes away: PDCs and messages from vPilot work without Hoppie. |
| `COM NOT AVAIL` | vPilot is open or was used, but isn't connected to VATSIM |
| `LINK LOST` | This screen has lost vDCDU. It reconnects by itself as soon as vDCDU is back. The hardware display goes dark after a minute without vDCDU. |
| `SEND FAILED` | Your reply didn't reach Hoppie. Press `SEND*` to try again. |
| `PRIORITY MSG` | A high-priority message is waiting behind the one you're answering |
| `NO MORE MSG` / `NO MORE PGE` | You're at the first or last message / page |

---

## Common problems

### The page doesn't open on the PC
vDCDU isn't running. Start it from the Start menu (**vDCDU**), or start the sim if it auto-starts with it. Then open exactly `http://localhost:8765`.

### The demo doesn't start
Only one copy of vDCDU can run at a time. End the running one first: Task Manager → **DCDU** → End task.

### Tablet can't open the page
Work through these in order:

1. The tablet is on the **same Wi-Fi network** as the PC, not a guest network.
2. You're using the right address: `http://` + the PC's **IPv4 Address** from `ipconfig` + `:8765`, e.g. `http://192.168.1.50:8765`.
3. **Windows Firewall:** vDCDU must be allowed on Private networks. Run the setup program again and tick **Let tablets and a Wi-Fi display connect**, or open **Windows Security → Firewall & network protection → Allow an app through firewall**, find **DCDU**, and tick **Private**.
4. **Your network must be "Private" in Windows.** Open **Settings → Network & internet**, click your Wi-Fi or Ethernet connection, and set **Network profile type** to **Private network**. On a "Public" network Windows blocks other devices from reaching the PC.

### vPilot shows "plugin not detected"
- Start vPilot after vDCDU, or restart it.
- vPilot's message area should show `DCDU Relay loaded` when it starts. If it doesn't, close vPilot and run the setup program again; check the vPilot folder it shows.
- The plugin keeps a log at `vPilot\Plugins\DcduRelay.log`.

### A CPDLC message takes a while to appear
Normal: Hoppie only allows checking for new messages every 60–90 seconds. After you reply, vDCDU checks every 20–30 seconds for 3 minutes.

### The Fenix and this DCDU both show the same message
Expected: they both receive it from Hoppie. Answer it on one only. You can hide the Fenix's DCDU in its EFB settings.

### No chime on the tablet
Browsers on iPad and Android only allow sound after you've touched the page. Tap anywhere on the page once after opening it.

### The tablet page freezes or shows old messages
Set the tablet's auto-lock / screen timeout to **Never** while flying. After the tablet wakes up, the page reconnects by itself within a few seconds.

### ATC info shows "All" even though I chose "My flight"
My flight needs your VATSIM flight plan. It shows once vPilot is connected and your flight plan is filed.

### The display doesn't connect
On the hardware display, `WAITING FOR DATA` with lines underneath tells you which stage it's stuck at (full list in the [DIY display guide](diy-display.md#what-the-waiting-screen-tells-you)):
- `CONNECT USB TO THE SIM PC`: the display is set to USB and vDCDU isn't talking to it. Check the cable carries data, vDCDU is running, and **Display → Look for a display on USB** is on. A board with a USB-serial chip (CH340, CP210x) must be ticked once under **Display → My display isn't listed**. The log shows `display on USB COM…` when it finds it. If the log says `can't open`, another program (Arduino IDE Serial Monitor, MobiFlight, a flasher) has the port: close it; vDCDU tries again when you re-plug the display.
- `WIFI <name> FAILED`: it can't join your Wi-Fi. Plug it into the PC by USB and set Wi-Fi again on the **Display** tab; the network must be 2.4 GHz.
- `SEARCHING FOR SIM PC` or `SIM PC <address>:8765` (the Display tab says **Wi-Fi: can't reach this PC**): it's on Wi-Fi but can't reach vDCDU.
  - **Your network must be "Private" in Windows.** This is the most common cause: the firewall rule only covers Private networks. Open **Settings → Network & internet**, click your Ethernet or Wi-Fi connection, and set **Network profile type** to **Private network**.
  - Check vDCDU is running, and the firewall point under [Tablet can't open the page](#tablet-cant-open-the-page).
  - If the display is on a separate IoT/guest network or VLAN, that network must be allowed to reach the PC on port 8765.
  - The address the display uses is shown on the Display tab. vDCDU skips VPN and Tailscale addresses, and updates an address it gave the display when this PC's address changes. An address you typed in yourself is kept; clear the box and **Save** to go back to automatic.

### Can't upload firmware: the COM port is busy
vDCDU holds the display's USB port while it runs. Turn off **Display → Look for a display on USB** on the web page (or quit vDCDU), then upload. Turn it back on afterwards.

### A key on my display does nothing
Open the **Display** tab and press the key. If it doesn't light up there, it isn't reaching vDCDU: check its wiring (between the GPIO and GND) and its `PIN_KEY_…` line in `config.h`. Greyed-out keys aren't set in `config.h`. See the [DIY display guide](diy-display.md#keys-and-the-atc-msg-light).

---

## Log files

Your vDCDU files are in `%LOCALAPPDATA%\DCDU` (Start menu → **vDCDU files (logs and settings)**).

| File | What's in it |
|---|---|
| `dcdu_bridge.log` | Everything vDCDU does: connections, messages, errors. Kept small automatically: at 1 MB it starts a new file and keeps two old ones (`.log.1`, `.log.2`), so 3 MB at most. |
| `dcdu_print.txt` | Messages you saved with PRINT |
| `vPilot\Plugins\DcduRelay.log` (in your vPilot folder) | What the vPilot plugin passed on |

---

## Reporting a problem

[Open an issue on GitHub](https://github.com/mhawk1134/vDCDU/issues/new/choose) and include:

1. What you did, what you expected, and what happened instead.
2. The three status indicators (a screenshot is fine).
3. The relevant part of `dcdu_bridge.log`.

**Never post your Hoppie logon code**, the contents of `hoppie_logon.txt`, or a `config.h` with your Wi-Fi password.
