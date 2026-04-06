#pragma once
#include <cstdint>
#include <cstddef>

// ─────────────────────────────────────────────────────────────────────────────
// Platform abstraction: compiles for both Windows DLL and NTS-3 bare-metal
// ─────────────────────────────────────────────────────────────────────────────
#ifdef LOGUE_SDK_BUILD
  #include <unit_genericfx.h>
  #define MC_ALLOC(size)  sdram_alloc(size)
  #define MC_FREE(ptr)    sdram_free((const uint8_t*)(ptr))
#else
  #include <cstdlib>
  #define MC_ALLOC(size)  (new uint8_t[size])
  #define MC_FREE(ptr)    (delete[] (uint8_t*)(ptr))
#endif


enum EffectIdx : int {
  // NANO LOOP category
  kEffectCollage    = 0,
  kEffectReorder       = 1,
  kEffectDrift     = 2,
  // GRAINS category
  kEffectFog      = 3,
  kEffectUnderpass    = 4,
  kEffectRiff     = 5,
  // DEFECT category
  kEffectSpurs    = 6,
  kEffectIntersperse = 7,
  kEffectScatter       = 8,
  // DEFER category
  kEffectRearrange   = 9,
  kEffectSpace      = 10,
  kEffectCount     = 11
};
extern const char* kEffectNames[kEffectCount];

// ─────────────────────────────────────────────────────────────────────────────
// Parameter IDs
// ─────────────────────────────────────────────────────────────────────────────
enum ParamId : uint8_t {
  kParamEffect   = 0,
  kParamPreset   = 1,
  kParamActivity = 2,
  kParamRepeats  = 3,
  kParamShape    = 4,
  kParamMix      = 5,
  kParamFilter   = 6,
  kParamModDepth = 7,
  kParamCount
};

// ─────────────────────────────────────────────────────────────────────────────
// Shared constants
// ─────────────────────────────────────────────────────────────────────────────
static constexpr float  kSampleRate = 48000.f;
static constexpr size_t kBufLenSec  = 4;
static constexpr size_t kBufLen     = (size_t)(kSampleRate * kBufLenSec);
static constexpr float  kPi         = 3.14159265358979f;
static constexpr float  kTwoPi      = 2.f * kPi;

// ─────────────────────────────────────────────────────────────────────────────
// LUTs
// ─────────────────────────────────────────────────────────────────────────────
static constexpr int kLutSize  = 1024;
static constexpr int kHannSize = 1024;
extern float g_sinLut[kLutSize];
extern float g_hannLut[kHannSize];
void initSinLut();
void initHannLut();

inline float fastSin(float phase) {
  int idx = (int)(phase * kLutSize) & (kLutSize - 1);
  return g_sinLut[idx < 0 ? idx + kLutSize : idx];
}
inline float fastCos(float phase) { return fastSin(phase + 0.25f); }
inline float hannWindow(float t) {
  int idx = (int)(t * (kHannSize - 1));
  if (idx < 0) idx = 0;
  if (idx >= kHannSize) idx = kHannSize - 1;
  return g_hannLut[idx];
}

// ─────────────────────────────────────────────────────────────────────────────
// Utilities
// ─────────────────────────────────────────────────────────────────────────────
inline float clampf(float v, float lo, float hi) {
  return v < lo ? lo : (v > hi ? hi : v);
}
inline float lerpf(float a, float b, float t) { return a + t * (b - a); }
inline float norm1000(int32_t v) { return clampf((float)v / 1000.f, 0.f, 1.f); }
