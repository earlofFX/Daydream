# UNDERPASS

**NTS-3 kaoss pad kit — logue SDK genericfx unit**

Cyclical micro-loops that generate hypnotic, evolving drone textures. A short window of audio loops continuously while the loop length and/or filtering is slowly modulated — creating a sound that shifts and breathes over time. Two staggered read heads with Hann envelopes ensure completely gapless, click-free output even as the loop length changes.

Developer ID: `EARL` (`0x4541524C`)
Unit ID: `0x00000004`

---

## Presets

Selected via the **X axis** of the kaoss pad, left to right:

| Position | Name | Description |
|----------|------|-------------|
| Far left | OFF | Bypass — dry signal passed through |
| | A | LFO modulates loop length — the drone compresses and lengthens |
| | B | Sub-octave drone (0.5× speed) with slow resonant LPF sweep |
| | C | Resonant bandpass filter sweep across the drone |
| Far right | D | Envelope-triggered length modulation — louder input = longer loop |

## Controls

| Control | Parameter | Range | Effect |
|---------|-----------|-------|--------|
| X axis | PRESET | OFF / A / B / C / D | Selects drone modifier type |
| Depth knob | ACTIV | 0–100% | Depth of modifier. Quantised to nearest 10% |

### Activity detail by preset

| Preset | Low activity | High activity |
|--------|-------------|---------------|
| A | Barely any length variation | Deep, slow compression/lengthening |
| B | Subtle filter movement | Wide, dramatic LPF sweep |
| C | Narrow bandpass sweep | Wide, resonant bandpass sweep |
| D | Slight envelope response | Dramatic length changes on loud input |

## Behaviour

- Always on — effect processes audio continuously, no touch required
- Initialises at preset A, 50% activity
- LFO period ranges from 2 seconds (high activity) to 30 seconds (low activity)
- Loop length changes are slew-rate limited to prevent clicks from sudden LFO direction changes
- Filter coefficients updated every 64 samples to prevent per-sample discontinuities

## Technical

- Base loop length: 250ms
- Loop length range: 50ms – 2 seconds
- Read heads: 2 (staggered by half a loop for continuous output)
- SDRAM usage: 2 × 768KB (stereo delay buffers)

## Building

Place the project folder at `platform/nts-3_kaoss/underpass-genericfx/` within the logue-sdk directory and run:

```bash
./run_cmd.sh build nts-3_kaoss/underpass-genericfx
```
