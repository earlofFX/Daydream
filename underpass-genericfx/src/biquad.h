#pragma once
#include "params.h"
#include <cmath>

// ─────────────────────────────────────────────────────────────────────────────
// Biquad filter (Direct Form I)
// Supports LPF, HPF, BPF modes.
// ─────────────────────────────────────────────────────────────────────────────
class Biquad {
public:
  Biquad() { reset(); }

  void reset() {
    _z1 = _z2 = 0.f;
    _b0 = 1.f; _b1 = _b2 = _a1 = _a2 = 0.f;
  }

  // Low-pass filter
  void setLPF(float cutoffHz, float Q, float sr) {
    float w0  = kTwoPi * cutoffHz / sr;
    float cosw = cosf(w0);
    float sinw = sinf(w0);
    float alpha = sinw / (2.f * Q);
    float a0r   = 1.f / (1.f + alpha);
    _b0 = (1.f - cosw) * 0.5f * a0r;
    _b1 = (1.f - cosw) * a0r;
    _b2 = _b0;
    _a1 = -2.f * cosw * a0r;
    _a2 = (1.f - alpha) * a0r;
  }

  // Band-pass filter (constant skirt gain)
  void setBPF(float centerHz, float Q, float sr) {
    float w0    = kTwoPi * centerHz / sr;
    float sinw  = sinf(w0);
    float cosw  = cosf(w0);
    float alpha = sinw / (2.f * Q);
    float a0r   = 1.f / (1.f + alpha);
    _b0 =  (sinw * 0.5f) * a0r;
    _b1 =  0.f;
    _b2 = -_b0;
    _a1 = -2.f * cosw * a0r;
    _a2 = (1.f - alpha) * a0r;
  }

  inline float process(float x) {
    float y = _b0 * x + _z1;
    _z1 = _b1 * x - _a1 * y + _z2;
    _z2 = _b2 * x - _a2 * y;
    return y;
  }

private:
  float _b0, _b1, _b2, _a1, _a2;
  float _z1, _z2;
};
