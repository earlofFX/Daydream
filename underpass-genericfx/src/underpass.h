#pragma once
#include "params.h"
#include "delay_line.h"
#include "biquad.h"

// ─────────────────────────────────────────────────────────────────────────────
// UNDERPASS — cyclical micro-loops generate hypnotic drones with unique modifiers
//
// A short section of audio loops continuously. The loop length and/or
// filtering is modulated per-preset, generating a hypnotic drone texture.
//
// Click-free design:
//   - Loop wrap uses a Hann crossfade window (peaks at loop midpoint, tapers
//     to zero at both ends). Two read heads staggered by half a loop so one
//     is always near its peak, giving continuous gapless output.
//   - Filter coefficients updated only every 64 samples to prevent
//     per-sample state resets that cause discontinuities.
//   - Loop length changes are slew-rate limited so sudden LFO jumps don't
//     cause the read position to teleport outside the window.
//
// Preset A: drone sample length compresses and lengthens (LFO on loop length)
// Preset B: sub-octave drone (0.5x speed) with resonant LPF sweep
// Preset C: drone samples have resonant bandpass filters
// Preset D: envelope-triggered compressing and lengthening of drone length
//
// Activity: depth of each modifier
// ─────────────────────────────────────────────────────────────────────────────

class UnderpassEngine {
public:
  static constexpr int kNumHeads = 2; // staggered read heads for click-free wrap

  UnderpassEngine()
    : _sr(48000.f), _lfoPhase(0.f), _lfoRate(0.f)
    , _modDepth(0.5f), _baseLoopLen(0.f)
    , _targetLoopLen(0.f), _currentLoopLen(0.f)
    , _envFollow(0.f), _preset(0), _filterUpdateCount(0) {
    _readPos[0] = _readPos[1] = 0.f;
    _readSpeed[0] = _readSpeed[1] = 1.f;
  }

  void init(float sr, float* bufL, float* bufR) {
    _sr = sr;
    _lineL.init(bufL, kBufLen);
    _lineR.init(bufR, kBufLen);
    _filterL.reset(); _filterR.reset();
    setPreset(0);
    setActivity(0.5f);
  }

  void setPreset(int preset) {
    _preset = preset;
    _baseLoopLen    = _sr * 0.25f; // 250ms base loop
    _targetLoopLen  = _baseLoopLen;
    _currentLoopLen = _baseLoopLen;
    _lfoPhase       = 0.f;

    // Stagger the two heads by half a loop so one is always near its
    // envelope peak while the other is near zero — continuous output
    _readPos[0] = 0.f;
    _readPos[1] = _baseLoopLen * 0.5f;

    switch (preset) {
      case 0: // A: LFO on length, no filter
        _readSpeed[0] = _readSpeed[1] = 1.f;
        _filterL.reset(); _filterR.reset();
        break;
      case 1: // B: sub-octave + LPF sweep
        _readSpeed[0] = _readSpeed[1] = 0.5f;
        _filterL.setLPF(500.f, 3.f, _sr);
        _filterR.setLPF(500.f, 3.f, _sr);
        break;
      case 2: // C: BPF — resonant bandpass
        _readSpeed[0] = _readSpeed[1] = 1.f;
        _filterL.setBPF(400.f, 4.f, _sr);
        _filterR.setBPF(400.f, 4.f, _sr);
        break;
      case 3: // D: envelope-triggered length modulation, no filter
        _readSpeed[0] = _readSpeed[1] = 1.f;
        _filterL.reset(); _filterR.reset();
        break;
    }
  }

  void setActivity(float norm) {
    _modDepth = norm;
    // LFO rate: 0.05 Hz (very slow, 20sec cycle) → 0.5 Hz (2sec cycle)
    _lfoRate  = lerpf(0.05f, 0.5f, norm) / _sr;
  }

  void process(const float* in, float* out, uint32_t frames) {
    for (uint32_t f = 0; f < frames; ++f) {
      float inL = in[f * 2];
      float inR = in[f * 2 + 1];
      _lineL.write(inL);
      _lineR.write(inR);

      // ── LFO ──────────────────────────────────────────────────────────────
      _lfoPhase += _lfoRate;
      if (_lfoPhase >= 1.f) _lfoPhase -= 1.f;
      float lfo = fastSin(_lfoPhase); // -1..1

      // ── Compute target loop length per preset ─────────────────────────────
      switch (_preset) {
        case 0: // A: LFO modulates loop length
          _targetLoopLen = _baseLoopLen * (1.f + lfo * _modDepth * 0.6f);
          break;
        case 1: // B: fixed length, LFO sweeps filter — update filter every 64 samples
          _targetLoopLen = _baseLoopLen;
          if ((_filterUpdateCount & 63) == 0) {
            float cutoff = 80.f + (lfo * 0.5f + 0.5f) * 3000.f * _modDepth;
            _filterL.setLPF(clampf(cutoff, 80.f, 6000.f), 3.f, _sr);
            _filterR.setLPF(clampf(cutoff, 80.f, 6000.f), 3.f, _sr);
          }
          break;
        case 2: // C: fixed length, LFO sweeps BPF centre
          _targetLoopLen = _baseLoopLen;
          if ((_filterUpdateCount & 63) == 0) {
            float centre = 150.f + (lfo * 0.5f + 0.5f) * 1500.f * _modDepth;
            _filterL.setBPF(clampf(centre, 150.f, 3000.f), 4.f, _sr);
            _filterR.setBPF(clampf(centre, 150.f, 3000.f), 4.f, _sr);
          }
          break;
        case 3: // D: envelope follower drives length
        {
          float envIn  = fabsf(inL) + fabsf(inR);
          float att    = 0.002f, rel = 0.0002f;
          float coef   = (envIn > _envFollow) ? att : rel;
          _envFollow  += coef * (envIn - _envFollow);
          _targetLoopLen = _baseLoopLen * (1.f + _envFollow * _modDepth * 1.2f);
        }
          break;
      }
      _filterUpdateCount++;

      // ── Slew-rate limit the loop length ───────────────────────────────────
      // Maximum change per sample: 0.5 samples. This prevents the read
      // position from suddenly falling outside the window when the LFO
      // changes direction, which was the main source of clicks.
      float maxChange = 0.5f;
      float diff = _targetLoopLen - _currentLoopLen;
      if (diff >  maxChange) diff =  maxChange;
      if (diff < -maxChange) diff = -maxChange;
      _currentLoopLen += diff;
      _currentLoopLen = clampf(_currentLoopLen, _sr * 0.05f, _sr * 2.f);

      // ── Two staggered read heads with Hann envelope ───────────────────────
      float sumL = 0.f, sumR = 0.f;

      for (int h = 0; h < kNumHeads; ++h) {
        // Advance read position
        _readPos[h] += _readSpeed[h];
        if (_readPos[h] >= _currentLoopLen) _readPos[h] -= _currentLoopLen;

        // Hann envelope: peaks at loop midpoint, zero at both ends.
        // With two heads offset by half a loop, one is always at peak.
        float t   = _readPos[h] / _currentLoopLen; // 0..1
        float env = hannWindow(t);

        // Read from the delay line: readPos=0 reads at the very start of
        // the loop window (farthest back), readPos=loopLen reads near-present.
        // We invert: delay = loopLen - readPos so that pos=0 → read oldest.
        float delay = _currentLoopLen - _readPos[h];
        delay = clampf(delay, 1.f, (float)kBufLen - 1.f);

        sumL += _lineL.readHermite(delay) * env;
        sumR += _lineR.readHermite(delay) * env;
      }

      // Two heads, each peaks at 1.0, so normalise by 2
      sumL *= 0.5f;
      sumR *= 0.5f;

      // Apply filter (no-op if reset — Biquad::reset() leaves passthrough state)
      sumL = _filterL.process(sumL);
      sumR = _filterR.process(sumR);

      out[f * 2]     = sumL;
      out[f * 2 + 1] = sumR;
    }
  }

private:
  float      _sr;
  float      _lfoPhase, _lfoRate;
  float      _modDepth;
  float      _baseLoopLen;
  float      _targetLoopLen, _currentLoopLen;
  float      _readPos[kNumHeads];
  float      _readSpeed[kNumHeads];
  float      _envFollow;
  int        _preset;
  int        _filterUpdateCount;
  Biquad     _filterL, _filterR;
  DelayLine  _lineL, _lineR;
};
