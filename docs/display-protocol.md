# Display protocol

How vDCDU talks to a hardware display, for anyone building a display that doesn't use the vDCDU firmware. The [firmware source](../firmware/DCDU_Display/) is a complete, working example (`dcdu_link.cpp`).

The idea: **vDCDU on the PC holds all the DCDU logic.** A display only draws the screen it's sent and sends key presses back. That's why every display and the web page always show the same thing.

---

## Connections

A display can connect in either of two ways, or both. It sends and receives the same messages on both.

### USB serial

- Newline-delimited JSON at 115200 baud (an ESP32-S3's own USB port ignores the baud rate). One message per line; lines that don't start with `{` are ignored, so a display can print log lines on the same port.
- vDCDU looks for displays on ports with USB vendor ID `0x303A` (Espressif) by itself, and on other ports only when the user ticks them on the Display tab.
- vDCDU opens the port with **DTR and RTS low**, so an ESP32 isn't reset. It sends `{"type":"ping"}`. The display must answer with a `hello` (below) whose `id` starts with `dcdu-` within **4 seconds**. A port that doesn't is left alone until it's unplugged.
- vDCDU sends `ping` every 5 seconds. A display can treat 10 seconds without any message as "link lost".

### WebSocket (Wi-Fi)

- `ws://<pc>:8765/ws`. Send a `hello` after connecting.
- vDCDU advertises itself with mDNS as `_dcdu._tcp` (port in the service record), so a display can find the PC by itself.
- vDCDU sends a `status` message at least every 30 seconds, and answers standard WebSocket ping frames.
- Over the WebSocket a display also gets the messages meant for web pages (`msg`, `status`, `settings` and others). Ignore any `type` you don't know.

When a display is connected both ways, show what arrives over USB.

---

## Display → vDCDU

### `hello`

Send it when the link comes up, and again whenever something in it changes.

```json
{"type": "hello", "id": "dcdu-7a6944", "name": "dcdu", "fw": "0.3.0",
 "board": "Guition JC4827W543C", "keys": ["L1", "L2", "R1", "R2"],
 "mode": "wifi", "wifi": "up", "ssid": "HomeNet", "ip": "192.168.1.124",
 "host": "", "port": 8765}
```

| Field | Meaning |
|---|---|
| `id` | **Required**, starts with `dcdu-`, unique per display (the firmware uses the last 3 bytes of the MAC address). |
| `name` | A short name, shown on the Display tab when it isn't `dcdu`. |
| `fw` | Firmware version, shown on the Display tab. |
| `board` | Board name, shown on the Display tab. |
| `keys` | The keys that are wired up (names as in `key` below). The Display tab greys out the others. Leave it out if unknown. |
| `mode` | `usb` or `wifi`: how the display is set to connect. |
| `wifi` | `off`, `joining`, `up` or `failed`. |
| `ssid` | The saved Wi-Fi network (empty if none). |
| `ip` | The display's address on Wi-Fi (empty if not connected). |
| `host`, `port` | The PC address the display uses on Wi-Fi (`""` = finds it itself). |

Only `id` is needed for the screen and keys to work. The Wi-Fi fields are for the Display tab's Wi-Fi setup; leave them out if your display doesn't do Wi-Fi.

### `key`

```json
{"type": "key", "key": "R2"}
```

Keys: `L1 L2 R1 R2` (line keys, top and bottom on each side), `MSGM MSGP` (MSG− / MSG+), `PGEM PGEP` (PGE− / PGE+), `BRT DIM PRINT ATCMSG`. Send it when a key is pressed down.

---

## vDCDU → display

### `screen` (and `state`)

```json
{"type": "screen", "s": { ... }}
```

Draw `s` and keep it until the next one. Over the WebSocket the first one arrives inside `{"type": "state", "screen": { ... }}`. A screen is sent whenever anything on it changes.

| Key | Meaning |
|---|---|
| `pwr` | `off`: blank screen. `test` / `wait`: powering up; draw only `ln` (no lines, prompts or header). `on`: normal. |
| `hdr` | Header, top left, green, small font (e.g. `1454Z FROM KZNY`). |
| `st` | Top right status field: `[text, style]`. Styles: `c` cyan, `g` green, `w` white small font, `bc` black on a cyan box, `bg` black on a green box. `null` = none. |
| `ln` | Up to 5 message lines. Each line is a list of `[text, colour]` spans drawn one after the other. Colours: `w` white, `g` green, `c` cyan, `a` amber, `k` black. Lines are at most 30 characters. |
| `sys` | Status line, bottom centre: `[text, colour]` or `null`. |
| `msg` | `"2/4"`: message 2 of 4, drawn as `MSG` over `2 / 4` (empty = hide). |
| `pg` | `"1/2"`: page 1 of 2, drawn as `PG` over `1 / 2` (empty = hide). |
| `k` | Prompts by key: `{"R2": ["WILCO", true, false]}` = `[label, active, pressed]`. An active prompt gets a `*` on the key side (`WILCO*` on the right, `*UNABLE` on the left). `pressed`: draw it inverted (black on cyan). |
| `lamp` | The ATC MSG light should flash. |
| `chime` | A counter: when it goes up and `lamp` is true, play the new-message chime (if your display has a speaker). |
| `br` | Brightness in percent (BRT / DIM). |
| `id` | The shown message's id (for the web page; displays can ignore it). |

If the link to vDCDU is lost, show `LINK LOST` in amber as the status line.

**Layout.** The firmware and the web page draw the screen at 480×272 with two monospace fonts: 14 px per character for the message lines and status field, 11 px for the header, prompts and centre column. The exact positions are the constants at the top of `dcdu.cpp` (the web page uses the same numbers).

### Wi-Fi setup (USB only)

vDCDU sends these only over USB, so a wrong Wi-Fi setting can never cut a display off from the Display tab.

| Message | Display does |
|---|---|
| `{"type":"wifi_scan"}` | Scan for networks, answer `{"type":"wifi_scan_result","ok":true,"networks":[{"ssid":"HomeNet","rssi":-60,"secure":true}]}` |
| `{"type":"wifi_set","ssid":"…","pass":"…"}` | Try to join (about 15 s). Answer `{"type":"wifi_result","ok":true,"ssid":"…","ip":"192.168.1.124"}`, or `"ok":false` with `"err"` (a short reason). On success, save it and switch to Wi-Fi mode; on failure keep the old settings. |
| `{"type":"set_mode","mode":"usb"}` / `"wifi"` | Switch modes (the saved network is kept) |
| `{"type":"set_host","host":"192.168.1.50","port":8765}` | Use this PC address on Wi-Fi (`""` = find it with mDNS) |
| `{"type":"wifi_forget"}` | Erase the saved network and go back to USB mode |

Send a new `hello` after each of these.

---

## Trying it out

Start vDCDU in [demo mode](user-guide.md#demo-mode): it sends a test message of every kind every 25 seconds. The web page's Display tab shows your display's `hello` details and the keys it sends. The log file (`dcdu_bridge.log`) shows when a display connects.
