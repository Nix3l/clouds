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

    f32 a1 = rand_float(&hash) * 2 * PI;
    f32 a2 = rand_float(&hash) * 2 * PI;

    return yaw_pitch_to_direction(a1, a2);
}

f32 perlin_noise_3d(f32 x, f32 y, f32 z) {
    // coordinates of the bottom left corner of the cell
    i32 ux = (i32) floorf(x);
    i32 uy = (i32) floorf(y);
    i32 uz = (i32) floorf(z);

    // coordinates of the point within the cell
    f32 rx = x - floorf(x);
    f32 ry = y - floorf(y);
    f32 rz = z - floorf(z);

    v3i offsets[8] = {
        V3I(0, 0, 0),
        V3I(1, 0, 0),
        V3I(0, 1, 0),
        V3I(1, 1, 0),
        V3I(0, 0, 1),
        V3I(1, 0, 1),
        V3I(0, 1, 1),
        V3I(1, 1, 1),
    };


    f32 prod[8];

    for(u32 i = 0; i < ARRAY_SIZE(offsets); i ++) {
        v3f relative_position = V3F(rx, ry, rz);
        v3f distance = relative_position;

        v3i cell_offset = offsets[i];
        if(cell_offset.x == 1) distance.x = 1.0f - rx;
        if(cell_offset.y == 1) distance.y = 1.0f - ry;
        if(cell_offset.z == 1) distance.z = 1.0f - rz;

        v3f grad = hash_gradient(ux + cell_offset.x, uy + cell_offset.y, uz + cell_offset.z);
        prod[i]  = glms_dot(glms_normalize(grad), glms_normalize(distance));
    }

    f32 fx = fade(rx);
    f32 fy = fade(ry);
    f32 fz = fade(rz);

    f32 x0 = lerp(prod[0], prod[1], fx);
    f32 x1 = lerp(prod[2], prod[3], fx);
    f32 x2 = lerp(x0, x1, fy);

    f32 y0 = lerp(prod[4], prod[5], fy);
    f32 y1 = lerp(prod[6], prod[7], fy);
    f32 y2 = lerp(y0, y1, fx);

    f32 res = lerp(x2, y2, fz);
    return (res + 1.0f) / 2.0f;
}
