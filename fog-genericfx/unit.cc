/*
    BSD 3-Clause License
    Copyright (c) 2023, KORG INC.

    File: unit.cc
    NTS-3 kaoss pad kit — FOG generic effect unit
    (granular clouds — clusters of grains create a wash of sound)

    Preset mapping (X axis, 0-4):
      0 = OFF — bypass, dry signal passed through
      1 = A   — short diffused wash, 1x speed, slow position scatter
      2 = B   — many randomised grains, widest scatter and duration range
      3 = C   — 1x and 2x speed grain mix (adds octave-up shimmer)
      4 = D   — 1x and 0.5x speed grain mix (adds octave-down depth)

    Activity (depth knob, 0-1023):
      Quantised to nearest 10%. Controls grain density:
      low = 2 simultaneous grains (sparse), high = 12 grains (dense wash).

    Always on — granular engine runs continuously, grains keep spawning.
    Initialises at preset A, 50% activity (~7 simultaneous grains).
*/

#include "unit_genericfx.h"
#include <climits>
#include "src/params.h"
#include "src/granules.h"

class Effect {
public:
  Effect() : _preset(1), _bypassed(false), _activityParam(511) {}
  ~Effect() {}

  inline int8_t Init(const unit_runtime_desc_t *desc) {
    if (!desc)
      return k_unit_err_undef;

    if (desc->target != unit_header.common.target)
      return k_unit_err_target;

    if (!UNIT_API_IS_COMPAT(desc->api))
      return k_unit_err_api_version;

    if (desc->samplerate != 48000)
      return k_unit_err_samplerate;

    if (desc->input_channels != 2 || desc->output_channels != 2)
      return k_unit_err_geometry;

    if (!desc->hooks.sdram_alloc)
      return k_unit_err_memory;

    const size_t kBytes = kBufLen * sizeof(float);
    float *bufL = (float *)desc->hooks.sdram_alloc(kBytes);
    if (!bufL) return k_unit_err_memory;
    float *bufR = (float *)desc->hooks.sdram_alloc(kBytes);
    if (!bufR) return k_unit_err_memory;

    initSinLut();
    initHannLut();

    _grains.init(48000.f, bufL, bufR);

    // Initialise at preset A (engine preset 0), 50% activity
    _preset        = 1;
    _bypassed      = false;
    _activityParam = 511;
    _grains.setPreset(0);
    _grains.setActivity(_quantiseActivity(511));

    return k_unit_err_none;
  }

  inline void Teardown() {}
  inline void Reset()    {}
  inline void Resume()   {}
  inline void Suspend()  {}

  inline void Process(const float *in, float *out, uint32_t frames) {
    if (_bypassed) {
      for (uint32_t f = 0; f < frames; ++f) {
        out[f * 2]     = in[f * 2];
        out[f * 2 + 1] = in[f * 2 + 1];
      }
    } else {
      _grains.process(in, out, frames);
    }
  }

  inline void setParameter(uint8_t id, int32_t value) {
    switch (id) {
      case 0: {
        // PRESET: 0=OFF, 1-4 = engine presets A-D
        _preset   = clipminmaxi32(0, value, 4);
        _bypassed = (_preset == 0);
        if (!_bypassed)
          _grains.setPreset(_preset - 1);  // map 1-4 → engine 0-3
        break;
      }
      case 1: {
        // ACTIV: quantise to nearest 10%
        _activityParam = clipminmaxi32(0, value, 1023);
        _grains.setActivity(_quantiseActivity(_activityParam));
        break;
      }
      default:
        break;
    }
  }

  inline int32_t getParameterValue(uint8_t id) const {
    switch (id) {
      case 0: return _preset;
      case 1: return _activityParam;
      default: return INT_MIN;
    }
  }

  inline const char *getParameterStrValue(uint8_t id, int32_t value) const {
    if (id == 0) {
      static const char *names[] = {"OFF", "A", "B", "C", "D"};
      if (value >= 0 && value <= 4) return names[value];
    }
    return nullptr;
  }

  inline void setTempo(uint32_t tempo)         { (void)tempo; }
  inline void tempo4ppqnTick(uint32_t counter) { (void)counter; }
  inline void touchEvent(uint8_t id, uint8_t phase, uint32_t x, uint32_t y) {
    (void)id; (void)phase; (void)x; (void)y;
  }

private:
  // Round activity to nearest 10% (0, 0.1, 0.2 ... 1.0)
  static inline float _quantiseActivity(int32_t raw) {
    const float norm      = (float)raw / 1023.f;
    const float quantised = (float)(int)(norm * 10.f + 0.5f) / 10.f;
    return (quantised < 0.f) ? 0.f : (quantised > 1.f) ? 1.f : quantised;
  }

  GrainEngine _grains;
  int         _preset;
  bool        _bypassed;
  int32_t     _activityParam;
};

// ---- Static instance --------------------------------------------------------

static Effect s_effect_instance;

// ---- Callbacks --------------------------------------------------------------

__unit_callback int8_t unit_init(const unit_runtime_desc_t *desc) {
  return s_effect_instance.Init(desc);
}
__unit_callback void unit_teardown()  { s_effect_instance.Teardown(); }
__unit_callback void unit_reset()     { s_effect_instance.Reset(); }
__unit_callback void unit_resume()    { s_effect_instance.Resume(); }
__unit_callback void unit_suspend()   { s_effect_instance.Suspend(); }
__unit_callback void unit_render(const float *in, float *out, uint32_t frames) {
  s_effect_instance.Process(in, out, frames);
}
__unit_callback void unit_set_param_value(uint8_t id, int32_t value) {
  s_effect_instance.setParameter(id, value);
}
__unit_callback int32_t unit_get_param_value(uint8_t id) {
  return s_effect_instance.getParameterValue(id);
}
__unit_callback const char *unit_get_param_str_value(uint8_t id, int32_t value) {
  return s_effect_instance.getParameterStrValue(id, value);
}
__unit_callback void unit_set_tempo(uint32_t tempo) {
  s_effect_instance.setTempo(tempo);
}
__unit_callback void unit_tempo_4ppqn_tick(uint32_t counter) {
  s_effect_instance.tempo4ppqnTick(counter);
}
__unit_callback void unit_touch_event(uint8_t id, uint8_t phase, uint32_t x, uint32_t y) {
  s_effect_instance.touchEvent(id, phase, x, y);
}
