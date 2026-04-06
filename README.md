# KORG NTS-3 - Daydream Effects

A collection of custom effects for the **KORG NTS-3 kaoss pad kit**, built using the [logue SDK](https://github.com/korginc/logue-sdk).

These effects are inspired by the lush, textural character of modern "dreamy" boutique effects pedals — granular washes, harmonic micro-loops, hypnotic drones, and spacious multi-tap delays. The goal is to bring that ambient, exploratory sound design aesthetic to the NTS-3's four effect slots.

> **These units were developed with the assistance of [Claude](https://claude.ai) (Anthropic's AI assistant), which was used throughout the DSP design, C++ implementation, and logue SDK integration process.**

---

## Units

| Unit | Effect Type | Character |
|------|-------------|-----------|
| [COLLAGE](collage/) | Micro-loop | Overlapping loops at harmonic speed ratios — octave stacking and shimmer |
| [REARRANGE](rearrange/) | Multi-tap delay | Four rhythmic tap configurations — straight, syncopated, triplet, double-time |
| [UNDERPASS](underpass/) | Drone loop | Cyclical micro-loops with slow modulation — hypnotic, evolving textures |
| [FOG](fog/) | Granular | Continuous grain clouds — sparse shimmer to dense atmospheric wash |
| [SPACE](space/) | Tonal delay | Per-tap filtering and pitch — delay tails that evolve tonally repeat by repeat |

---

## Common Controls

All five units share the same control scheme on the NTS-3 kaoss pad:

| Control | Function |
|---------|----------|
| **X axis** | Preset selection — sweep left to right through OFF / A / B / C / D |
| **Depth knob** | Activity — controls density, number of heads/taps/grains |
| **Y axis** | Tempo (REARRANGE and SPACE only) |

All units are **always on** — the effect runs continuously and does not require the pad to be touched to process audio. Touching the pad adjusts parameters. All units initialise at **preset A, 50% activity**.

Activity is quantised to the nearest 10%, preventing small finger movements from changing the sound unintentionally during performance. Note that some effects require a couple of seconds of buffered audio before stabilising.  


---

## Installation

### Pre-built (recommended)

Download the `.nts3unit` files and load them onto your NTS-3 using the [KORG KONTROL Editor]

### Building from source

You will need the [logue SDK](https://github.com/korginc/logue-sdk) and the ARM embedded toolchain (`gcc-arm-none-eabi`).

1. Clone this repository
2. Copy the unit folder you want to build into `platform/nts-3_kaoss/` inside your logue-sdk directory
3. Build using the SDK wrapper:

```bash
./run_cmd.sh build nts-3_kaoss/collage-genericfx
```

4. Load the resulting `.nts3unit` file via KORG KONTROL Editor


---

## Developer

Developer ID: **EARL** (`0x4541524C`)

| Unit ID | Unit |
|---------|------|
| `0x00000002` | COLLAGE |
| `0x00000003` | REARRANGE |
| `0x00000004` | UNDERPASS |
| `0x00000005` | FOG |
| `0x00000006` | SPACE |

---

## Disclaimer

**These units are provided as-is, with no warranty of any kind, expressed or implied.**

Use at your own risk. The author accepts no liability whatsoever for any damage to hardware, data loss, unexpected behaviour, or any other consequence arising from the use of these units. Loading custom firmware onto your NTS-3 is done entirely at your own risk and may void your warranty — refer to KORG's documentation before proceeding.

These units are not affiliated with, endorsed by, or supported by KORG Inc.

---

## Licence

Source code is released under the [GNU General Public License v3.0](https://www.gnu.org/licenses/gpl-3.0.en.html).

This means you are free to use, modify, and distribute this code, but any derivative works must also be released under GPL v3 with source code made available.

Note that the logue SDK itself is released under the BSD 3-Clause Licence by KORG Inc. The NTS-3 kaoss pad kit is a product of KORG Inc.
