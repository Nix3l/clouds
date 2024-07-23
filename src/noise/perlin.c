#include "perlin.h"

#include "game.h"
#include "util/log.h"
#include "util/math.h"

// improved fade function
// 6t^5 -15t^4 + 10t^3
static f32 fade(f32 t) {
    return t*t*t*(t*(6.0f*t-15.0f)+10.0f);
}

static f32 lerp(f32 t0, f32 t1, f32 w) {
    return t0 + (t1 - t0) * w;
}

static u32 lcg_int(u32 seed) {
    return (seed * 1664525 + 1013904223) % MAX_u32;
}

static f32 rand_float(u32* hash) {
    u32 seed = *hash;
    f32 res = (f32) (seed) / (f32) MAX_u32;
    *hash = lcg_int(seed);
    return res;
}

// https://www.beosil.com/download/CollisionDetectionHashing_VMV03.pdf
static v3f hash_gradient(i32 x, i32 y, i32 z) {
    u32 hash = x * 73856093;
    hash ^= y * 19349663;
    hash ^= z * 83492791;
    hash %= MAX_u32;

    f32 a1 = rand_float(&hash);
    f32 a2 = rand_float(&hash);

    return yaw_pitch_to_direction(a1, a2);
}

f32 perlin_noise_3d(f32 x, f32 y, f32 z) {
    v3f grad = hash_gradient(x, y, z);
    return RAND_IN_RANGE(0.0f, 1.0f);
}
