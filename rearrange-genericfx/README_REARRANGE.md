# REARRANGE

**NTS-3 kaoss pad kit — logue SDK genericfx unit**

A multi-tap delay with four distinct rhythmic tap configurations. Each preset arranges the delay taps into a different rhythmic pattern — from straight quarter-note repeats through syncopated dotted-eighth feels, triplet grids, and double-time patterns. The Y axis controls tempo, so you can sweep BPM live as you play.

Developer ID: `EARL` (`0x4541524C`)
Unit ID: `0x00000003`

---

## Presets

Selected via the **X axis** of the kaoss pad, left to right:

| Position | Name | Description |
|----------|------|-------------|
| Far left | OFF | Bypass — dry signal passed through |
| | A | Classic linear — taps at 1, 2, 3, 4 beats |
| | B | Syncopated — dotted-eighth feel with stereo spread |
| | C | Triplet grid — taps at triplet subdivisions |
| Far right | D | Double time — taps at every half beat (up to 4 beats) |

## Controls

| Control | Parameter | Range | Effect |
|---------|-----------|-------|--------|
| X axis | PRESET | OFF / A / B / C / D | Selects tap rhythm pattern |
| Depth knob | ACTIV | 0–100% | Number of active taps (1–8). Quantised to nearest 10% |
| Y axis | TEMPO | 40–200 BPM | Sets tap spacing. Overridden by external MIDI clock if connected |

## Behaviour

- Always on — effect processes audio continuously, no touch required
- Initialises at preset A, 50% activity (~4 taps), 120 BPM
- Activity is quantised to the nearest 10% to prevent micro-jitter
- External MIDI clock automatically overrides the manual tempo parameter
- Later taps decay in level — the delay tail fades naturally

## Technical

- Max taps: 8
- Tempo range: 40–200 BPM (manual), or external MIDI clock
- SDRAM usage: 2 × 768KB (stereo delay buffers)

## Building

Place the project folder at `platform/nts-3_kaoss/rearrange-genericfx/` within the logue-sdk directory and run:

```bash
./run_cmd.sh build nts-3_kaoss/rearrange-genericfx
```
