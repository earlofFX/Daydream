# SPACE

**NTS-3 kaoss pad kit — logue SDK genericfx unit**

A multi-tap delay where each individual tap is processed with a different tonal or pitch manipulation before summing. Unlike REARRANGE (which varies the rhythmic arrangement of taps), SPACE uses fixed linear tap spacing and applies a distinct character to each tap — creating a delay tail that evolves tonally from repeat to repeat.

Developer ID: `EARL` (`0x4541524C`)
Unit ID: `0x00000006`

---

## Presets

Selected via the **X axis** of the kaoss pad, left to right:

| Position | Name | Description |
|----------|------|-------------|
| Far left | OFF | Bypass — dry signal passed through |
| | A | LPF per tap — earlier taps bright, later taps progressively darker and warmer |
| | B | Resonant BPF per tap — each tap resonates at a different overtone frequency |
| | C | Pitch-shifted taps — alternating fifth up (1.5×) and fourth down (0.75×) |
| Far right | D | Shimmer — later taps increasingly blend in octave-up (2×) content |

## Controls

| Control | Parameter | Range | Effect |
|---------|-----------|-------|--------|
| X axis | PRESET | OFF / A / B / C / D | Selects per-tap processing type |
| Depth knob | ACTIV | 0–100% | Number of active taps (1–8). Quantised to nearest 10% |
| Y axis | TEMPO | 40–200 BPM | Sets tap spacing. Overridden by external MIDI clock if connected |

### Preset character detail

| Preset | Tap 1 | Tap 4 | Tap 8 |
|--------|-------|-------|-------|
| A (LPF) | 4000Hz cutoff (bright) | 2800Hz cutoff | 700Hz cutoff (dark) |
| B (BPF) | 200Hz resonance | 800Hz resonance | 1600Hz resonance |
| C (Pitch) | 1.5× (up a fifth) | 0.75× (down a fourth) | 1.5× (up a fifth) |
| D (Shimmer) | 20% octave blend | 50% octave blend | 90% octave blend |

## Behaviour

- Always on — effect processes audio continuously, no touch required
- Initialises at preset A, 50% activity (~4 taps), 120 BPM
- Activity is quantised to the nearest 10% to prevent micro-jitter
- External MIDI clock automatically overrides the manual tempo parameter
- All presets use linear tap spacing — the interest is in tonal evolution, not rhythmic variation

## Technical

- Max taps: 8
- Tap spacing: linear (1, 2, 3 ... 8 beats)
- Tempo range: 40–200 BPM (manual), or external MIDI clock
- SDRAM usage: 2 × 768KB (stereo delay buffers)

## Building

Place the project folder at `platform/nts-3_kaoss/space-genericfx/` within the logue-sdk directory and run:

```bash
./run_cmd.sh build nts-3_kaoss/space-genericfx
```
