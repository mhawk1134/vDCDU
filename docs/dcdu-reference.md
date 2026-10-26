# vDCDU vs the real DCDU

vDCDU emulates the DCDU (Datalink Control and Display Unit) found in Airbus cockpits. You can use it while flying any aircraft. Its behaviour follows the open-source FlyByWire A32NX DCDU and published descriptions of the real unit.

## Same as the real DCDU

- Screen layout: header, 5 message lines of 30 characters, prompts beside the four line keys, status line, `MSG n / m`, `PG n / m`
- Colours: green header, white text, cyan values and prompts, amber faults
- Keys: line keys, MSG−/+, PGE−/+, BRT/DIM, PRINT, ATC MSG light
- Long messages in pages; reply keys locked until every page is read
- CPDLC answers in two steps (choose, then SEND); STBY sent at once
- `CLOSE*` and `RECALL*`
- Power-up: `SELF TEST IN PROGRESS`, then `WAITING FOR DATA`
- Idle: `CURRENT ATC UNIT <station> CTL` after a CPDLC logon

## Added by vDCDU

- PDCs, private messages, frequency text, SELCAL, broadcasts and ATIS from vPilot and VATSIM
- Runs in a web browser, with a message log, ATC info list and filters
- MSG−/+ in recall mode browse older messages
- PRINT saves to a text file
- "Power" comes from the sim running or vPilot being connected

## Not included (yet)

- Sending requests or logons to ATC (use your aircraft's ACARS / MCDU or another Hoppie client)
- The `*OTHER` prompt, reminders and message monitoring
