# How vDCDU compares to the real DCDU

vDCDU aims to work like the A320's DCDU (Datalink Control and Display Unit) as closely as practical. This page lists the rules it follows, what it adds, and what it leaves out.

**Sources:** the open-source FlyByWire A32NX DCDU (built with input from type-rated pilots), FlyByWire's Hoppie guide, and published descriptions and photos of the real unit. Airbus's own manuals weren't available, so details marked *(FBW)* follow FlyByWire's implementation.

---

## The unit

- **Keys:** BRT / DIM (top left), MSG− / MSG+ (left), PRINT (top right), PGE− / PGE+ (right), and two line keys on each side at the bottom (L1, L2, R1, R2), next to the prompts.
- **ATC MSG** pushbutton on the glareshield: flashes, with a chime, when an ATC message arrives; the DCDU shows the message by itself. Pressing ATC MSG stops the flashing.

## The screen

| Zone | Content |
|---|---|
| Header, top left | `1443Z FROM EDYY CTL`, small green font |
| Status field, top right | The message's status (below) |
| Message area | 5 lines of 30 characters |
| Separators | A white line across at about 65 % height; two white lines down at about 26 % and 74 % width, splitting the bottom into left prompts, centre and right prompts |
| Centre column | The status line (top), `MSG n / m` (more than one message), `PG n / m` (more than one page) |
| Prompts | L1 / L2 at the left edge, R1 / R2 at the right, small cyan font. An active prompt has a `*` on the key side: `*UNABLE`, `WILCO*` |

## Colours

| Element | Colour |
|---|---|
| Header | green |
| `OPEN` (answer needed) | cyan |
| Answer chosen, not sent | black on a cyan box: `WILCO`, `UNABLE`, `STBY`, `ROGER`, `AFFIRM`, `NEGATV` |
| Answer sent | black on a green box |
| Message from ATC | white |
| Values ATC filled in (levels, frequencies, waypoints) | cyan |
| Message after it's answered (except STANDBY) | green, values no longer highlighted |
| Prompts | cyan; a pressed key's prompt shows black on cyan for a moment |
| Separator lines | white |
| Status line | white; amber for faults (`LINK LOST`, `SEND FAILED`, …) |
| Nothing to show | `CURRENT ATC UNIT <station> CTL` in green with the station cyan, once a logon is accepted; otherwise empty |

## Pages and messages *(FBW)*

- Text is word-wrapped at 30 characters. Page 1 shows 5 lines; each later page shows the last line of the previous page again at the top, then 4 new lines.
- PGE− / PGE+ step through pages; at either end the status line shows `NO MORE PGE`.
- **The reply keys are locked until every page has been read.**
- MSG− / MSG+ step through open messages; at either end `NO MORE MSG`.
- High-priority messages start with `***HIGH PRIORITY***`. One waiting behind the message you're answering shows `PRIORITY MSG`.
- `CLOSE*` removes a message. With nothing open, `RECALL*` (R2) brings back the last closed one (`RECALL MODE`).

## Replies *(FBW: two steps, choose then SEND)*

| Message needs | Before answering | After choosing | After sending |
|---|---|---|---|
| WILCO / UNABLE | L1 `*UNABLE`, R1 `STBY*`, R2 `WILCO*` | L1 `*CANCEL`, R2 `SEND*` | R2 `CLOSE*` |
| AFFIRM / NEGATIVE | L1 `*NEGATV`, R2 `AFFIRM*` | `*CANCEL` / `SEND*` | `CLOSE*` |
| ROGER | R2 `ROGER*` | `*CANCEL` / `SEND*` | `CLOSE*` |
| No reply | R2 `CLOSE*` | | |

STBY is sent at once and the message stays open, with WILCO / UNABLE still offered.

## Power

Power-up shows `SELF TEST IN PROGRESS (MAX 10 SECONDS)`, then `WAITING FOR DATA (MAX 30 SECONDS)`, then the normal screen. On power loss the screen stays for a few seconds, then goes blank. vDCDU follows FlyByWire's timings (6 s, 12 s, 10 s). Since it can't see the aircraft's electrics yet, "power" means: Microsoft Flight Simulator is running, or vPilot is connected. Messages are kept.

---

## What vDCDU adds

Not on the real unit, marked as additions wherever they appear in these docs:

- **Other messages in the same frame:** US pre-departure clearances (vTDLS, laid out like an airline PDC with the values cyan), private messages, frequency text, SELCAL, network broadcasts, ATIS and controller info, each with a small type label and `CLOSE*`.
- **Recall browsing:** in recall mode, MSG− / MSG+ step through older closed messages.
- **PRINT** adds the message to a text file on the PC instead of printing.
- **The web page:** a message log (tap a message to show it), the ATC info list, filters, and several screens showing the same DCDU.
- Status line `COM NOT INIT` without a Hoppie code, `COM NOT AVAIL` while vPilot is offline.

## Not modelled (yet)

- Requests from the pilot (downlinks from the MCDU) and logons to ATC: do these from your aircraft's MCDU.
- The `*OTHER` prompt, reminders for high-priority messages, and message monitoring.
- Real aircraft power: the DCDU follows the sim and vPilot instead.

## Answering on one DCDU only

vDCDU reads the same Hoppie messages as your aircraft (for example the Fenix). Answer each message on one DCDU only, or the controller gets two answers. The Fenix can hide its own DCDU in its EFB settings.
