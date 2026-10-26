# Troubleshooting

Check the indicators under the DCDU first. Green = working, grey = off or waiting, amber = needs attention.

## Indicators

| Indicator | Says | Do |
|---|---|---|
| vPilot | `not running` | Start vPilot |
| | `plugin not detected` | Close vPilot, run the setup program again, start vPilot |
| | `running, not connected` | Connect to VATSIM |
| | `connected <callsign>` | Working |
| Hoppie | `off (no logon code)` | Fine without CPDLC. For CPDLC, run setup again and add your code. |
| | `error: …` | Wrong logon code: run setup again and retype it |
| | `paused` | vPilot isn't connected |
| | `ok <callsign>` | Working |
| VATSIM data | `unreachable` | Check your internet; it retries |

## The DCDU is dark

Normal until the sim is running or vPilot is connected.

## Status line messages

| Message | Meaning |
|---|---|
| `COM NOT AVAIL` | vPilot isn't connected to VATSIM |
| `COM NOT INIT` | No vPilot connection and no Hoppie code |
| `LINK LOST` | This screen lost vDCDU. It reconnects by itself. |
| `SEND FAILED` | Your CPDLC answer didn't reach Hoppie. Press `SEND*` again. |

## The page doesn't open

Start vDCDU from the Start menu (or start the sim), then open `http://localhost:8765`.

## Tablet can't connect

1. Tablet and PC on the same network.
2. Address is `http://<PC's IPv4 address>:8765`.
3. Windows must call your network **Private**: **Settings → Network & internet →** your connection **→ Private network**.
4. Run setup again and tick **Let tablets and a Wi-Fi display connect**.

## No chime

Click or tap the page once after opening it.

## The demo won't start

vDCDU is already running. End it first: Task Manager → **DCDU** → End task.

## Hardware display

See [Build your own display](diy-display.md#if-it-doesnt-connect).

## Log files

Start menu → **vDCDU files**: `dcdu_bridge.log`. The vPilot plugin logs to `Plugins\DcduRelay.log` in your vPilot folder.

## Still stuck?

[Open an issue](https://github.com/mhawk1134/vDCDU/issues/new/choose) with a screenshot of the indicators and the relevant part of `dcdu_bridge.log`. **Never post your Hoppie logon code.**
