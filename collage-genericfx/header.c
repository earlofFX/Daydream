/*
    BSD 3-Clause License
    Copyright (c) 2023, KORG INC.

    File: header.c
    NTS-3 kaoss pad kit — COLLAGE generic effect unit header

*/

#include "unit_genericfx.h"

// ---- Unit header definition -------------------------------------------------

const __unit_header genericfx_unit_header_t unit_header = {
  .common = {
    .header_size = sizeof(genericfx_unit_header_t),
    .target      = UNIT_TARGET_PLATFORM | k_unit_module_genericfx,
    .api         = UNIT_API_VERSION,
    .dev_id      = 0x4541524C,   // EARL of FX
    .unit_id     = 0x00000002U,
    .version     = 0x00010000U,
    .name        = "COLLAGE",
    .num_params  = 2,

    .params = {
      // Format: min, max, center, default, type, frac_bits, frac_mode, reserved, name

      // PRESET: 0=OFF, 1=A, 2=B, 3=C, 4=D
      // X axis sweeps across all 5 positions left to right
      {0, 4, 0, 1, k_unit_param_type_none, 0, 0, 0, {"PRESET"}},

      // ACTIV: loop head density, quantised to nearest 10%
      // Depth knob
      {0, 1023, 511, 511, k_unit_param_type_none, 0, 0, 0, {"ACTIV"}},

      // Unused placeholders — must fill to 8 total
      {0, 0, 0, 0, k_unit_param_type_none, 0, 0, 0, {""}},
      {0, 0, 0, 0, k_unit_param_type_none, 0, 0, 0, {""}},
      {0, 0, 0, 0, k_unit_param_type_none, 0, 0, 0, {""}},
      {0, 0, 0, 0, k_unit_param_type_none, 0, 0, 0, {""}},
      {0, 0, 0, 0, k_unit_param_type_none, 0, 0, 0, {""}},
      {0, 0, 0, 0, k_unit_param_type_none, 0, 0, 0, {""}}
    },
  },

  .default_mappings = {
    // PRESET → X axis (0=OFF, 1=A .. 4=D, left to right)
    {k_genericfx_param_assign_x,     k_genericfx_curve_linear, k_genericfx_curve_unipolar, 0, 4,    1},

    // ACTIV → depth knob, default 50%
    {k_genericfx_param_assign_depth, k_genericfx_curve_linear, k_genericfx_curve_unipolar, 0, 1023, 511},

    // Unused slots
    {k_genericfx_param_assign_none, k_genericfx_curve_linear, k_genericfx_curve_unipolar, 0, 0, 0},
    {k_genericfx_param_assign_none, k_genericfx_curve_linear, k_genericfx_curve_unipolar, 0, 0, 0},
    {k_genericfx_param_assign_none, k_genericfx_curve_linear, k_genericfx_curve_unipolar, 0, 0, 0},
    {k_genericfx_param_assign_none, k_genericfx_curve_linear, k_genericfx_curve_unipolar, 0, 0, 0},
    {k_genericfx_param_assign_none, k_genericfx_curve_linear, k_genericfx_curve_unipolar, 0, 0, 0},
    {k_genericfx_param_assign_none, k_genericfx_curve_linear, k_genericfx_curve_unipolar, 0, 0, 0}
  }
};
