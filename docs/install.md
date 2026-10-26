# Installation guide

This sets vDCDU up on your flight sim PC. Each step says what to do, what you should see, and what to do if you don't. Plan on about five minutes.

---

## Before you start

- [ ] A **Windows 10 or 11** PC with Microsoft Flight Simulator 2020 or 2024.
- [ ] **vPilot** installed and working with your VATSIM account.
- [ ] A **Hoppie ACARS logon code**. If you don't have one, register at [hoppie.nl](https://www.hoppie.nl/acars/system/register.html); the code arrives by email within a few minutes. If you fly the Fenix, it's the code already in your Fenix EFB.
- [ ] Optional: a tablet on the **same network** as the PC.

No Python or other programs needed: the setup program has everything.

---

## Step 1: Download

Go to the [latest release](https://github.com/mhawk1134/vDCDU/releases/latest) and download **`vDCDU-Setup-<version>.exe`** (under *Assets*).

## Step 2: Run the setup program

1. **Close vPilot** if it's running (the setup program installs a small plugin into it).
2. Double-click the downloaded file.

**Windows SmartScreen** may say *"Windows protected your PC"*, because vDCDU is new and not code-signed yet. Click **More info**, then **Run anyway**.

The setup program asks:

| Page | What to do |
|---|---|
| **Licence** | Read it, choose *I accept*. |
| **Hoppie ACARS logon code** | Type your code. You can leave it empty and add it later by running the setup program again. |
| **vPilot** | It finds your vPilot folder by itself (the one with `vPilot.exe`). Check it, or leave it empty if you don't use vPilot. |
| **Tasks** | **Start vDCDU with Microsoft Flight Simulator**: recommended. vDCDU then starts and stops with the sim, with no window.<br>**Let tablets and a Wi-Fi display connect**: tick it if you'll use a tablet or a Wi-Fi display. Windows asks for permission to add a firewall rule. |

If Windows has your network set to **Public**, the setup program offers to set it to **Private**. Say **Yes** for your home network: on a Public network Windows blocks tablets and Wi-Fi displays.

**You should see:** *Setup has finished installing vDCDU*, with **Start vDCDU now** ticked. Click **Finish**.

**If the last page says *"vDCDU is installed, but:"*** it lists what didn't work and what to do about it.

## Step 3: Open the DCDU

With **Start vDCDU now** ticked, your browser opens the DCDU at **http://localhost:8765**. Later, start it from the Start menu (**vDCDU**), or let the sim start it.

**You should see:** the DCDU in a grey bezel, dark, with three status indicators below it. That's right: like the real one, the DCDU only gets power while the sim is running or vPilot is connected. Tap or click once anywhere on the page, so the new-message chime can play (browsers block sound until you do).

## Step 4: Check it before your first flight

Start MSFS and vPilot and connect to VATSIM. The DCDU powers up (`SELF TEST IN PROGRESS`, then `WAITING FOR DATA`, about 20 seconds). Under the DCDU you should see:

| Indicator | Should say |
|---|---|
| **Hoppie** | `ok <your callsign>` |
| **vPilot** | `connected <your callsign> (plugin …)` |
| **VATSIM data** | `ok` (or `not loaded yet` until it's first needed) |

vPilot's message area also shows `DCDU Relay loaded` when it starts. Anything else? The [troubleshooting guide](troubleshooting.md) explains every status.

> **Fenix pilots:** the Fenix receives the same CPDLC messages. Answer each message on **one** DCDU only, or hide the Fenix's own DCDU in its EFB settings.

Next: the [user guide](user-guide.md) explains the keys and the web page. Want to try everything without flying first? See [Demo mode](user-guide.md#demo-mode).

---

## On a tablet

1. On the sim PC, find its address: Start menu → type `PowerShell` → open it → type `ipconfig` and press Enter. Note the **IPv4 Address**, for example `192.168.1.50`.
2. On the tablet (on the same Wi-Fi as the PC), open Safari or Chrome and go to `http://192.168.1.50:8765`, using your own address.
3. Optional: **Share → Add to Home Screen** (iPad) or **⋮ → Add to Home screen** (Android) for a full-screen DCDU icon.

**If the tablet can't connect**, see [Troubleshooting → Tablet can't open the page](troubleshooting.md#tablet-cant-open-the-page).

> **Tip:** give your PC a fixed address (a "DHCP reservation") in your router, so the tablet bookmark keeps working.

## A hardware display

See [Build your own display](diy-display.md).

---

## Where things are

| What | Where |
|---|---|
| The program | `%LOCALAPPDATA%\Programs\DCDU` |
| Your files: Hoppie code, settings, logs, printed messages | `%LOCALAPPDATA%\DCDU` (Start menu: **vDCDU files (logs and settings)**) |
| Auto-start with the sim | An entry called `DCDU Bridge` in the sim's `exe.xml` |
| vPilot plugin | `DcduRelay.dll` in vPilot's `Plugins` folder |

To open a folder, paste its path into the File Explorer address bar.

## Updating

Download the new setup program and run it. It closes vDCDU if it's running, keeps your Hoppie code, settings and choices, and doesn't ask the questions again. Close vPilot first if the release notes mention the plugin.

## Changing your Hoppie code or vPilot folder

Run the setup program again, or uninstall and install again.

## Uninstalling

**Settings → Apps → Installed apps → vDCDU → Uninstall.** This also removes the sim auto-start entry, the vPilot plugin and the firewall rule. It asks whether to delete your own files (Hoppie code, settings, logs) too.
