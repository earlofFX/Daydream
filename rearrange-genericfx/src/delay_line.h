#pragma once
#include "params.h"
#include <cstring>

// ─────────────────────────────────────────────────────────────────────────────
// Stereo interpolated delay line
// All delay-based effects use this.
// Buffer memory is owned externally (allocated via MC_ALLOC in unit_init).
// ─────────────────────────────────────────────────────────────────────────────
class DelayLine {
public:
  DelayLine() : _buf(nullptr), _len(0), _write(0) {}

  void init(float* buf, size_t len) {
    _buf   = buf;
    _len   = len;
    _write = 0;
    memset(buf, 0, len * sizeof(float));
  }

  // Write one sample to the head
  void write(float s) {
    _buf[_write] = s;
    _write = (_write + 1) % _len;
  }

  // Read back 'delaySamples' behind the write head (linear interp)
  float read(float delaySamples) const {
    float pos  = (float)_write - delaySamples - 1.f;
    // wrap into valid range
    while (pos < 0.f)         pos += (float)_len;
    while (pos >= (float)_len) pos -= (float)_len;

    int   i0   = (int)pos;
    float frac = pos - (float)i0;
    int   i1   = (i0 + 1) % (int)_len;

    return _buf[i0] + frac * (_buf[i1] - _buf[i0]);
  }

  // Hermite cubic interpolation (smoother for grain playback)
  float readHermite(float delaySamples) const {
    float pos = (float)_write - delaySamples - 1.f;
    while (pos < 0.f)          pos += (float)_len;
    while (pos >= (float)_len)  pos -= (float)_len;

    int   i1   = (int)pos;
    float frac = pos - (float)i1;
    int   i0   = (i1 - 1 + (int)_len) % (int)_len;
    int   i2   = (i1 + 1) % (int)_len;
    int   i3   = (i1 + 2) % (int)_len;

    float p0 = _buf[i0], p1 = _buf[i1], p2 = _buf[i2], p3 = _buf[i3];
    float c0 = p1;
    float c1 = 0.5f * (p2 - p0);
    float c2 = p0 - 2.5f * p1 + 2.f * p2 - 0.5f * p3;
    float c3 = 0.5f * (p3 - p0) + 1.5f * (p1 - p2);
    return ((c3 * frac + c2) * frac + c1) * frac + c0;
  }

  size_t length()    const { return _len;   }
  size_t writeHead() const { return _write; }

private:
  float*  _buf;
  size_t  _len;
  size_t  _write;
};
