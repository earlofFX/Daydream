# FOG

**NTS-3 kaoss pad kit — logue SDK genericfx unit**

A granular cloud effect that continuously fragments incoming audio into overlapping grains and reassembles them into a wash of sound. Grains are spawned at a rate calculated to maintain a target number of simultaneous active grains, guaranteeing a continuous wash rather than a stutter regardless of grain duration. Each grain has independent random position, duration, pan, and speed.

Developer ID: `EARL` (`0x4541524C`)
Unit ID: `0x00000005`

---

## Presets

Selected via the **X axis** of the kaoss pad, left to right:

| Position | Name | Description |
|----------|------|-------------|
| Far left | OFF | Bypass — dry signal passed through |
| | A | Short diffused wash — longer grains (200–400ms), 1× speed, slow scatter |
| | B | Dense randomised cloud — widest duration and scatter range |
| | C | 1× + 2× speed mix — adds octave-up shimmer to the wash |
| Far right | D | 1× + 0.5× speed mix — adds octave-down depth to the wash |

## Controls

| Control | Parameter | Range | Effect |
|---------|-----------|-------|--------|
| X axis | PRESET | OFF / A / B / C / D | Selects grain character |
| Depth knob | ACTIV | 0–100% | Grain density. Quantised to nearest 10% |

### Activity / density

| Activity | Simultaneous grains | Character |
|----------|--------------------|-----------| 
| 0% | ~2 grains | Sparse, stuttery |
| 50% | ~7 grains | Continuous wash |
| 100% | ~12 grains | Dense cloud |

## Behaviour

- Always on — granular engine runs continuously, grains keep spawning
- Initialises at preset A, 50% activity (~7 simultaneous grains)
- Activity is quantised to the nearest 10% — each step of the depth knob produces a clearly audible change in density
- Input audio is always being written to the buffer even when in bypass (OFF), so switching to an active preset immediately has material to work with
- Output level is normalised by the square root of active grain count to maintain consistent loudness at all densities

## Technical

- Max simultaneous grains: 16
- Grain duration: 150–500ms depending on preset
- Position scatter: up to 1 second of recent audio
- Grain pan: random stereo spread ±40%
- SDRAM usage: 2 × 768KB (stereo delay buffers)

## Building

Place the project folder at `platform/nts-3_kaoss/fog-genericfx/` within the logue-sdk directory and run:

```bash
./run_cmd.sh build nts-3_kaoss/fog-genericfx
```
