#pragma once
#include "params.h"
#include "delay_line.h"

// ─────────────────────────────────────────────────────────────────────────────
// COLLAGE / MICRO LOOP
//
// Multiple read heads play back from a shared circular record buffer at
// different speeds (pitch ratios). The key to a smooth sound is that each
// head uses a Hann crossfade window so its loop wrap point is inaudible.
//
// Each head maintains a phase (0..loopLen) and a Hann envelope derived from
// that phase. When phase wraps, the audio crossfades smoothly through zero
// rather than jumping — this eliminates clicks entirely.
//
// Preset A: 1x + 2x   (unison + octave up)
// Preset B: 1x + 0.5x (unison + octave down)
// Preset C: 2x only   (all active heads at double speed, octave up)
// Preset D: 0.5x + 1x + 2x + 4x (full harmonic stack)
//
// Activity: number of active heads (1→4). Levels are normalised so total
//           output level stays consistent regardless of head count.
// ─────────────────────────────────────────────────────────────────────────────

struct MicroLoopHead {
  float speed;    // playback speed (0.5, 1.0, 2.0, 4.0)
  float loopLen;  // loop window length in samples
  float phase;    // 0..loopLen, advances by speed each sample
  bool  active;
};

class MicroLoop {
public:
  static constexpr int kMaxHeads = 4;

  MicroLoop() : _sr(48000.f), _activeHeads(2), _preset(0) {}

  void init(float sr, float* bufL, float* bufR) {
    _sr = sr;
    _lineL.init(bufL, kBufLen);
    _lineR.init(bufR, kBufLen);
    setPreset(0);
  }

  void setPreset(int preset) {
    _preset = preset;

    // 400ms loop length — tight and rhythmic, matching Microcosm character.
    float loopLen = _sr * 0.4f;

    // Speed per head per preset
    static const float kSpeeds[4][4] = {
      { 1.f,  2.f,  1.f,  2.f  },  // A: 1x + 2x (pairs)
      { 1.f,  0.5f, 1.f,  0.5f },  // B: 1x + 0.5x (pairs)
      { 2.f,  2.f,  2.f,  2.f  },  // C: all 2x
      { 0.5f, 1.f,  2.f,  4.f  },  // D: full harmonic stack
    };

    for (int i = 0; i < kMaxHeads; ++i) {
      _heads[i].speed   = kSpeeds[preset][i];
      _heads[i].loopLen = loopLen;
      // Stagger phases so heads don't all reach their wrap points
      // simultaneously — prevents rhythmic pumping from coincident
      // envelope zeros
      _heads[i].phase  = loopLen * ((float)i / (float)kMaxHeads);
      _heads[i].active = (i < _activeHeads);
    }
  }

  // norm 0..1 → 1..4 active heads
  void setActivity(float norm) {
    _activeHeads = 1 + (int)(norm * (kMaxHeads - 1) + 0.5f);
    for (int i = 0; i < kMaxHeads; ++i)
      _heads[i].active = (i < _activeHeads);
  }

  void process(const float* in, float* out, uint32_t frames) {
    // Normalise level so output stays consistent as head count changes
    float level = 0.8f / (float)_activeHeads;

    for (uint32_t f = 0; f < frames; ++f) {
      _lineL.write(in[f * 2]);
      _lineR.write(in[f * 2 + 1]);

      float sumL = 0.f, sumR = 0.f;

      for (int h = 0; h < kMaxHeads; ++h) {
        MicroLoopHead& hd = _heads[h];
        if (!hd.active) continue;

        // Hann envelope: peaks at centre of loop, tapers to zero at both
        // ends. With phases staggered across heads, at least one head is
        // always near its peak — giving continuous, click-free output.
        float t   = hd.phase / hd.loopLen;  // 0..1
        float env = hannWindow(t);

        // Read position: phase 0 reads loopLen samples back from the
        // write head; phase loopLen reads right at the write head.
        // Faster speed → phase advances faster → higher perceived pitch.
        float delay = hd.loopLen - hd.phase;

        sumL += _lineL.readHermite(delay) * env * level;
        sumR += _lineR.readHermite(delay) * env * level;

        // Advance and wrap
        hd.phase += hd.speed;
        if (hd.phase >= hd.loopLen) hd.phase -= hd.loopLen;
      }

      out[f * 2]     = sumL;
      out[f * 2 + 1] = sumR;
    }
  }

private:
  float          _sr;
  int            _preset;
  int            _activeHeads;
  MicroLoopHead  _heads[kMaxHeads];
  DelayLine      _lineL, _lineR;
};
