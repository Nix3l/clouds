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

// https://www.beosil.com/download/CollisionDetectionHashing_VMV03.pdf
static u32 cell_hash(i32 x, i32 y, i32 z) {
    u32 hash;

    hash  = x * 73856093;
    hash ^= y * 19349663;
    hash ^= z * 83492791;
    hash %= MAX_u32;
    
    return hash;
}

// idk perlin made this ask him
static f32 grad(i32 hash, f32 x, f32 y, f32 z) {
    i32 h = hash & 15;
    f32 u = h < 8 ? x : y;
    
    f32 v = h < 4 ? y : h == 12 || h == 14 ? x : z;
    
    return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
}

f32 perlin_noise_3d(f32 x, f32 y, f32 z) {
    i32 cx = (i32) floorf(x);
    i32 cy = (i32) floorf(y);
    i32 cz = (i32) floorf(z);

    f32 rx = x - floorf(x);
    f32 ry = y - floorf(y);
    f32 rz = z - floorf(z);

    f32 fx = fade(rx);
    f32 fy = fade(ry);
    f32 fz = fade(rz);

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

    f32 dotp[8];
    for(usize i = 0; i < 8; i ++) {
        v3f rpos = V3F(rx, ry, rz);

        v3i cell_offset = offsets[i];
        if(cell_offset.x == 1) rpos.x = 1.0f - rx;
        if(cell_offset.y == 1) rpos.y = 1.0f - ry;
        if(cell_offset.z == 1) rpos.z = 1.0f - rz;

        i32 hash = cell_hash(cx + cell_offset.x, + cy + cell_offset.y, + cz + cell_offset.z);
        dotp[i] = grad(hash, rpos.x, rpos.y, rpos.z);
    }

    f32 x1 = lerp(dotp[0], dotp[1], fx);
    f32 x2 = lerp(dotp[2], dotp[3], fx);
    f32 y1 = lerp(x1, x2, fy);

    f32 x3 = lerp(dotp[4], dotp[5], fx);
    f32 x4 = lerp(dotp[6], dotp[7], fx);
    f32 y2 = lerp(x3, x4, fy);

    return (lerp(y1, y2, fz) + 1.0f) / 2.0f;
}
