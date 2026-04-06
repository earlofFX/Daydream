#include "params.h"
#include <cmath>

float g_sinLut[kLutSize];
float g_hannLut[kHannSize];

void initSinLut() {
  for (int i = 0; i < kLutSize; ++i)
    g_sinLut[i] = sinf(kTwoPi * (float)i / (float)kLutSize);
}

void initHannLut() {
  for (int i = 0; i < kHannSize; ++i) {
    float t = (float)i / (float)(kHannSize - 1);
    g_hannLut[i] = 0.5f * (1.f - cosf(kTwoPi * t));
  }
}
