#pragma once
#include "params.h"
#include "delay_line.h"
#include <cstdlib>  // rand()

// ─────────────────────────────────────────────────────────────────────────────
// FOG — clusters of grains create a wash of sound
//
// Key design principle for a continuous wash:
//   spawn interval = grain duration / target overlap count
// This guarantees N grains are always active simultaneously regardless of
// duration, which is what separates a wash from a stutter.
//
// Preset A: short, diffused — stretching overlapping samples
//           Longer grains (200–400ms), slow scatter, 1x speed
// Preset B: many simultaneous randomised grains — diffused textural effects
//           Wide duration range, heavy position scatter, highest density
// Preset C: mixture of normal and double speed grains — diffused texture
//           Some grains at 1x, some at 2x, spread positions
// Preset D: mixture of normal and half speed grains — diffused texture
//           Some grains at 1x, some at 0.5x, spread positions
//
// Activity: controls grain density (target overlap count 2..12)
// ─────────────────────────────────────────────────────────────────────────────

struct Grain {
  float pos;       // read position (samples behind write head) at spawn
  float duration;  // grain length in samples
  float speed;     // playback speed (affects how fast we move through the grain)
  float panL;
  float panR;
  float age;       // 0..duration
  bool  active;
};

class GrainEngine {
public:
  static constexpr int kMaxGrains = 16; // 48 for windows increased to support dense overlap

  GrainEngine() : _sr(48000.f), _spawnAccum(0.f), _preset(0) {}

  void init(float sr, float* bufL, float* bufR) {
    _sr = sr;
    _lineL.init(bufL, kBufLen);
    _lineR.init(bufR, kBufLen);
    for (int i = 0; i < kMaxGrains; ++i) _grains[i].active = false;
    setPreset(0);
    setActivity(0.5f);
  }

  void setPreset(int preset) {
    _preset = preset;
    switch (preset) {
      case 0:
        // A: short diffused — longer grains, slow position, 1x only
        _grainDurMin = 0.20f; _grainDurMax = 0.40f;
        _speedA = 1.f; _speedB = 1.f;
        _posScatterSec = 0.3f;
        break;
      case 1:
        // B: many randomised grains — widest duration and scatter range
        _grainDurMin = 0.15f; _grainDurMax = 0.50f;
        _speedA = 1.f; _speedB = 1.f;
        _posScatterSec = 1.0f; // scatter over 1 second of recent audio
        break;
      case 2:
        // C: 1x + 2x speed mix
        _grainDurMin = 0.15f; _grainDurMax = 0.35f;
        _speedA = 1.f; _speedB = 2.f;
        _posScatterSec = 0.5f;
        break;
      case 3:
        // D: 1x + 0.5x speed mix
        _grainDurMin = 0.20f; _grainDurMax = 0.40f;
        _speedA = 1.f; _speedB = 0.5f;
        _posScatterSec = 0.5f;
        break;
    }
    // Recompute spawn rate for new durations
    updateSpawnRate();
  }

  void setActivity(float norm) {
    // Target overlap: 2 grains at 0.0 → 12 grains at 1.0
    // More overlap = denser, more continuous wash
    _targetOverlap = 2.f + norm * 10.f;
    updateSpawnRate();
  }

  void process(const float* in, float* out, uint32_t frames) {
    for (uint32_t f = 0; f < frames; ++f) {
      _lineL.write(in[f * 2]);
      _lineR.write(in[f * 2 + 1]);

      // Spawn grains at rate that maintains target overlap
      _spawnAccum += _spawnRate;
      if (_spawnAccum >= 1.f) {
        _spawnAccum -= 1.f;
        spawnGrain();
      }

      float sumL = 0.f, sumR = 0.f;
      int   active = 0;

      for (int g = 0; g < kMaxGrains; ++g) {
        Grain& gr = _grains[g];
        if (!gr.active) continue;

        float t   = gr.age / gr.duration;
        float env = hannWindow(t);

        // Read forward through the captured grain window at playback speed.
        // pos is the delay at grain start; we read progressively closer to
        // the write head as age increases (speed=1 reads in real time,
        // speed=2 moves through the window twice as fast = pitch up).
        float readDelay = gr.pos - gr.age * gr.speed;
        if (readDelay < 1.f) readDelay = 1.f;

        sumL += _lineL.readHermite(readDelay) * env * gr.panL;
        sumR += _lineR.readHermite(readDelay) * env * gr.panR;
        active++;

        gr.age += 1.f;
        if (gr.age >= gr.duration) gr.active = false;
      }

      // Normalise by active grain count so output level stays consistent
      // regardless of density — prevents the wash from clipping at high activity

      // Instead of sqrtf((float)active):
      float scale = (active > 0) ? (0.7f / __builtin_sqrtf((float)active)) : 0.f;
      // float scale = (active > 0) ? (0.7f / sqrtf((float)active)) : 0.f;
      out[f * 2]     = sumL * scale;
      out[f * 2 + 1] = sumR * scale;
    }
  }

private:
  void updateSpawnRate() {
    // Average grain duration in samples
    float avgDurSamples = ((_grainDurMin + _grainDurMax) * 0.5f) * _sr;
    // Spawn one grain every (avgDur / targetOverlap) samples
    // This guarantees ~targetOverlap grains are always active
    float spawnInterval = avgDurSamples / _targetOverlap;
    _spawnRate = 1.f / spawnInterval;
  }

  void spawnGrain() {
    int idx = findFreeGrain();
    if (idx < 0) return; // pool full — already at max density

    Grain& g = _grains[idx];
    g.active = true;
    g.age    = 0.f;

    // Random duration within preset range
    float durSec = lerpf(_grainDurMin, _grainDurMax, frand());
    g.duration = durSec * _sr;

    // Position: anchor in recent audio + random scatter backward in time.
    // Minimum delay = duration so we always read within a captured window.
    float scatter = frand() * _posScatterSec * _sr;
    g.pos = g.duration + scatter;
    g.pos = clampf(g.pos, g.duration, (float)kBufLen * 0.85f);

    // Speed: random choice between the two configured speeds
    g.speed = (frand() > 0.5f) ? _speedA : _speedB;

    // Pan: random spread across stereo field
    float pan = lerpf(-0.4f, 0.4f, frand());
    //g.panL = sqrtf(clampf(0.5f - pan, 0.f, 1.f));
    //g.panR = sqrtf(clampf(0.5f + pan, 0.f, 1.f));

    // Instead of sqrtf(clampf(...)):
    g.panL = __builtin_sqrtf(clampf(0.5f - pan, 0.f, 1.f));
    g.panR = __builtin_sqrtf(clampf(0.5f + pan, 0.f, 1.f));
  }

  int findFreeGrain() {
    for (int i = 0; i < kMaxGrains; ++i)
      if (!_grains[i].active) return i;
    return -1;
  }

  static float frand() { return (float)rand() / (float)RAND_MAX; }

  float      _sr;
  int        _preset;
  float      _spawnRate;
  float      _spawnAccum;
  float      _targetOverlap;
  float      _grainDurMin, _grainDurMax;
  float      _speedA, _speedB;
  float      _posScatterSec;
  Grain      _grains[kMaxGrains];
  DelayLine  _lineL, _lineR;
};
