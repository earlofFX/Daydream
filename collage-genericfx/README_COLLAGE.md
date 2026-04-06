# COLLAGE

**NTS-3 kaoss pad kit — logue SDK genericfx unit**

Overlapping micro-loops played back at different speeds simultaneously, creating layered harmonic textures from your input signal. Each loop head uses a Hann crossfade window so wrap points are inaudible — the output is continuous and click-free regardless of how many heads are active.

Developer ID: `EARL` (`0x4541524C`)
Unit ID: `0x00000002`

---

## Presets

Selected via the **X axis** of the kaoss pad, left to right:

| Position | Name | Description |
|----------|------|-------------|
| Far left | OFF | Bypass — dry signal passed through |
| | A | 1× + 2× speed (unison + octave up) |
| | B | 1× + 0.5× speed (unison + octave down) |
| | C | 2× only (all heads at double speed — octave up) |
| Far right | D | 0.5× + 1× + 2× + 4× (full harmonic stack) |

## Controls

| Control | Parameter | Range | Effect |
|---------|-----------|-------|--------|
| X axis | PRESET | OFF / A / B / C / D | Selects loop speed configuration |
| Depth knob | ACTIV | 0–100% | Number of active loop heads (1–4). Quantised to nearest 10% |

## Behaviour

- Always on — effect processes audio continuously, no touch required
- Initialises at preset A, 50% activity (2 active heads)
- Activity is quantised to the nearest 10% to prevent micro-jitter from changing the sound during performance
- Output level is normalised as head count changes, so adding more heads doesn't increase volume

## Technical

- Loop length: 400ms
- Max simultaneous heads: 4
- SDRAM usage: 2 × 768KB (stereo delay buffers)
- Tempo: not applicable

## Building

Place the project folder at `platform/nts-3_kaoss/collage-genericfx/` within the logue-sdk directory and run:

```bash
./run_cmd.sh build nts-3_kaoss/collage-genericfx
```
