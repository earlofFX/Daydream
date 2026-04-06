#pragma once
#include "params.h"
#include "delay_line.h"
#include "biquad.h"

// ─────────────────────────────────────────────────────────────────────────────
// REARRANGE — multi-tap delay with four rhythmic tap configurations
//
// Activity: number of active taps (1..8)
// Preset selects the rhythmic pattern of tap positions.
//
// A: classic linear delay (taps at 1, 2, 3, 4 beats)
// B: syncopated / dotted-8th feel
// C: triplet grid
// D: double-time (taps at every half beat)
//
// Tap timing is tempo-relative. Without external tempo, defaults to 120 BPM.
// ─────────────────────────────────────────────────────────────────────────────

struct DelayTap {
  float fraction;  // delay time as multiple of one beat
  float panL;
  float panR;
  float level;
};

static const DelayTap kPatternTaps[4][8] = {
  // A: classic, linear
  { {1.f,0.7f,0.7f,0.80f},{2.f,0.7f,0.7f,0.60f},
    {3.f,0.7f,0.7f,0.40f},{4.f,0.7f,0.7f,0.20f},
    {0},{0},{0},{0} },
  // B: syncopated dotted-8th
  { {0.75f,0.8f,0.4f,0.80f},{1.5f,0.4f,0.8f,0.70f},
    {2.25f,0.8f,0.4f,0.60f},{3.0f,0.4f,0.8f,0.50f},
    {0},{0},{0},{0} },
  // C: triplet grid
  { {0.667f,0.8f,0.5f,0.80f},{1.333f,0.5f,0.8f,0.70f},
    {2.000f,0.8f,0.5f,0.60f},{2.667f,0.5f,0.8f,0.50f},
    {3.333f,0.8f,0.5f,0.40f},{4.000f,0.5f,0.8f,0.30f},
    {0},{0} },
  // D: double time
  { {0.5f,0.7f,0.7f,0.80f},{1.0f,0.7f,0.7f,0.70f},
    {1.5f,0.7f,0.7f,0.60f},{2.0f,0.7f,0.7f,0.50f},
    {2.5f,0.7f,0.7f,0.40f},{3.0f,0.7f,0.7f,0.30f},
    {3.5f,0.7f,0.7f,0.20f},{4.0f,0.7f,0.7f,0.10f} },
};

class RearrangeDelay {
public:
  static constexpr int kMaxTaps = 8;

  RearrangeDelay() : _sr(48000.f), _beatLen(24000.f), _activeTaps(4), _preset(0) {}

  void init(float sr, float* bufL, float* bufR) {
    _sr = sr;
    _lineL.init(bufL, kBufLen);
    _lineR.init(bufR, kBufLen);
    setPreset(0);
  }

  void setPreset(int p) {
    _preset = p & 3;
    for (int i = 0; i < kMaxTaps; ++i)
      _taps[i] = kPatternTaps[_preset][i];
  }

  void setActivity(float norm) {
    _activeTaps = 1 + (int)(norm * (kMaxTaps - 1) + 0.5f);
  }

  void setTempo(uint32_t tempo_fp) {
    float bpm = (float)(tempo_fp >> 16) + (float)(tempo_fp & 0xFFFF) / 65536.f;
    if (bpm < 1.f) bpm = 120.f;
    _beatLen = _sr * 60.f / bpm;
  }

  void process(const float* in, float* out, uint32_t frames) {
    for (uint32_t f = 0; f < frames; ++f) {
      _lineL.write(in[f*2]);
      _lineR.write(in[f*2+1]);

      float sumL = 0.f, sumR = 0.f;
      int n = _activeTaps < kMaxTaps ? _activeTaps : kMaxTaps;

      for (int t = 0; t < n; ++t) {
        if (_taps[t].fraction == 0.f) continue;
        float delay = clampf(_taps[t].fraction * _beatLen, 1.f, (float)kBufLen - 1.f);
        sumL += _lineL.readHermite(delay) * _taps[t].panL * _taps[t].level;
        sumR += _lineR.readHermite(delay) * _taps[t].panR * _taps[t].level;
      }

      out[f*2]   = sumL;
      out[f*2+1] = sumR;
    }
  }

private:
  float     _sr, _beatLen;
  int       _activeTaps, _preset;
  DelayTap  _taps[kMaxTaps];
  DelayLine _lineL, _lineR;
};

// ─────────────────────────────────────────────────────────────────────────────
// SPACE — multi-tap delay with per-tap filter and pitch manipulation
//
// Same tap timing as Rearrange but every tap is processed individually.
// Activity: number of active taps (1..8)
//
// Preset A: envelope-controlled LPF on each tap
//           → each tap has a different fixed cutoff; higher taps are brighter
// Preset B: resonant BPF on each tap
//           → each tap resonates at a different frequency, creating a
//             comb-like tonal texture across the repeats
// Preset C: pitch-shifted taps
//           → alternating taps read at 1.5x and 0.75x speed, producing
//             intervals above and below the original
// Preset D: taps cross-fade with double-speed grains
//           → each tap blends between its normal read and a 2x-speed read,
//             adding shimmery octave-up content to the delay tail
// ─────────────────────────────────────────────────────────────────────────────

class SpaceDelay {
public:
  static constexpr int kMaxTaps = 8;

  SpaceDelay() : _sr(48000.f), _beatLen(24000.f), _activeTaps(4), _preset(0) {}

  void init(float sr, float* bufL, float* bufR) {
    _sr = sr;
    _lineL.init(bufL, kBufLen);
    _lineR.init(bufR, kBufLen);
    for (int i = 0; i < kMaxTaps; ++i) {
      _filtersL[i].reset();
      _filtersR[i].reset();
    }
    setPreset(0);
  }

  void setPreset(int p) {
    _preset = p & 3;
    // Use the same linear tap positions as PATTERN A for all SPACE presets —
    // the interest comes from the per-tap processing, not the rhythmic pattern
    for (int i = 0; i < kMaxTaps; ++i)
      _taps[i] = kPatternTaps[0][i]; // always linear spacing

    // Configure per-tap processing for each preset
    for (int i = 0; i < kMaxTaps; ++i) {
      _filtersL[i].reset();
      _filtersR[i].reset();
      _tapSpeed[i] = 1.f;

      switch (_preset) {
        case 0:
          // A: LPF per tap — cutoff decreases with tap index (earlier taps brighter)
          _tapCutoff[i] = 4000.f - (float)i * 400.f;
          _tapCutoff[i] = clampf(_tapCutoff[i], 300.f, 8000.f);
          _filtersL[i].setLPF(_tapCutoff[i], 1.5f, _sr);
          _filtersR[i].setLPF(_tapCutoff[i], 1.5f, _sr);
          break;

        case 1:
          // B: resonant BPF — each tap resonates at a different musical interval
          // Frequencies approximate overtone series: 200, 300, 400, 600, 800...
          _tapCutoff[i] = 200.f * (float)(i + 1);
          _tapCutoff[i] = clampf(_tapCutoff[i], 100.f, 6000.f);
          _filtersL[i].setBPF(_tapCutoff[i], 4.f, _sr);
          _filtersR[i].setBPF(_tapCutoff[i], 4.f, _sr);
          break;

        case 2:
          // C: alternating pitch shift — reads at different speeds
          // Even taps: 1.5x (up a fifth), odd taps: 0.75x (down a fourth)
          _tapSpeed[i] = (i % 2 == 0) ? 1.5f : 0.75f;
          break;

        case 3:
          // D: double-speed crossfade — blend normal + 2x read for shimmer
          // Blend amount increases with tap index (later taps more shimmer)
          _tapSpeed[i] = 2.f; // used as the second read speed
          _tapBlend[i] = 0.2f + (float)i * 0.1f; // 20%..90% shimmer
          _tapBlend[i] = clampf(_tapBlend[i], 0.f, 0.9f);
          break;
      }
    }
  }

  void setActivity(float norm) {
    _activeTaps = 1 + (int)(norm * (kMaxTaps - 1) + 0.5f);
  }

  void setTempo(uint32_t tempo_fp) {
    float bpm = (float)(tempo_fp >> 16) + (float)(tempo_fp & 0xFFFF) / 65536.f;
    if (bpm < 1.f) bpm = 120.f;
    _beatLen = _sr * 60.f / bpm;
  }

  void process(const float* in, float* out, uint32_t frames) {
    for (uint32_t f = 0; f < frames; ++f) {
      _lineL.write(in[f*2]);
      _lineR.write(in[f*2+1]);

      float sumL = 0.f, sumR = 0.f;
      int n = _activeTaps < kMaxTaps ? _activeTaps : kMaxTaps;

      for (int t = 0; t < n; ++t) {
        if (_taps[t].fraction == 0.f) continue;

        float baseDelay = clampf(_taps[t].fraction * _beatLen, 1.f, (float)kBufLen - 1.f);
        float sL, sR;

        switch (_preset) {
          case 0: // A: LPF — read normally, apply filter
            sL = _filtersL[t].process(_lineL.readHermite(baseDelay));
            sR = _filtersR[t].process(_lineR.readHermite(baseDelay));
            break;

          case 1: // B: BPF — read normally, apply resonant filter
            sL = _filtersL[t].process(_lineL.readHermite(baseDelay));
            sR = _filtersR[t].process(_lineR.readHermite(baseDelay));
            break;

          case 2: // C: pitch shift — read at different speeds
            // Simulate pitch shift by reading from a scaled delay position
            // (approximation: not a true pitch shifter, but gives the interval feel)
          {
            float shiftedDelay = clampf(baseDelay / _tapSpeed[t], 1.f, (float)kBufLen - 1.f);
            sL = _lineL.readHermite(shiftedDelay);
            sR = _lineR.readHermite(shiftedDelay);
          }
            break;

          case 3: // D: shimmer — crossfade normal + double-speed read
          {
            float normalDelay  = baseDelay;
            float shimmerDelay = clampf(baseDelay * 0.5f, 1.f, (float)kBufLen - 1.f);
            float blend = _tapBlend[t];
            sL = lerpf(_lineL.readHermite(normalDelay),
                       _lineL.readHermite(shimmerDelay), blend);
            sR = lerpf(_lineR.readHermite(normalDelay),
                       _lineR.readHermite(shimmerDelay), blend);
          }
            break;

          default:
            sL = _lineL.readHermite(baseDelay);
            sR = _lineR.readHermite(baseDelay);
        }

        sumL += sL * _taps[t].panL * _taps[t].level;
        sumR += sR * _taps[t].panR * _taps[t].level;
      }

      out[f*2]   = sumL;
      out[f*2+1] = sumR;
    }
  }

private:
  float     _sr, _beatLen;
  int       _activeTaps, _preset;
  DelayTap  _taps[kMaxTaps];
  Biquad    _filtersL[kMaxTaps];
  Biquad    _filtersR[kMaxTaps];
  float     _tapCutoff[kMaxTaps];
  float     _tapSpeed[kMaxTaps];
  float     _tapBlend[kMaxTaps];
  DelayLine _lineL, _lineR;
};

// ─────────────────────────────────────────────────────────────────────────────
// MultiDelay — thin wrapper so callers can hold one object per effect
// and route to RearrangeDelay or SpaceDelay as needed
// ─────────────────────────────────────────────────────────────────────────────
class MultiDelay {
public:
  void init(float sr, float* bufL, float* bufR) {
    _rearrange.init(sr, bufL, bufR);
    _space.init(sr, bufL, bufR);
  }
  void setPreset(int p)        { _rearrange.setPreset(p); _space.setPreset(p); }
  void setActivity(float norm) { _rearrange.setActivity(norm); _space.setActivity(norm); }
  void setTempo(uint32_t fp)   { _rearrange.setTempo(fp); _space.setTempo(fp); }

  void processRearrange(const float* in, float* out, uint32_t frames) {
    _rearrange.process(in, out, frames);
  }
  void processSpace(const float* in, float* out, uint32_t frames) {
    _space.process(in, out, frames);
  }

private:
  RearrangeDelay _rearrange;
  SpaceDelay    _space;
};
