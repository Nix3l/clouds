#ifndef PERLIN_NOISE_H
#define PERLIN_NOISE_H

#include "base.h"

// TODO(nix3l): seed
f32 perlin_noise_3d(f32 x, f32 y, f32 z);
f32 perlin_noise_3d_wrap(f32 x, f32 y, f32 z, i32 wrap);

#endif
