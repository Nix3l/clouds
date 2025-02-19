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

// idk perlin made this ask him
static f32 grad(i32 hash, f32 x, f32 y, f32 z) {
    i32 h = hash & 15;
    f32 u = h < 8 ? x : y;
    
    f32 v = h < 4 ? y : h == 12 || h == 14 ? x : z;
    
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

f32 perlin_noise_3d(f32 x, f32 y, f32 z) {
    i32 p[] = {
        151,160,137,91,90,15,
        131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
        190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
        88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
        77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
        102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
        135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
        5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
        223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
        129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
        251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
        49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
        138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
        151,160,137,91,90,15,
        131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
        190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
        88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
        77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
        102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
        135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
        5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
        223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
        129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
        251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
        49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
        138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
    };

    i32 cx = (i32) floorf(x) & 255;
    i32 cy = (i32) floorf(y) & 255;
    i32 cz = (i32) floorf(z) & 255;

    f32 rx = x - floorf(x);
    f32 ry = y - floorf(y);
    f32 rz = z - floorf(z);

    f32 fx = fade(rx);
    f32 fy = fade(ry);
    f32 fz = fade(rz);

    i32 aaa, aba, aab, abb, baa, bba, bab, bbb;
    aaa = p[p[p[cx    ] + cy    ] + cz    ];
    aba = p[p[p[cx    ] + cy + 1] + cz    ];
    aab = p[p[p[cx    ] + cy    ] + cz + 1];
    abb = p[p[p[cx    ] + cy + 1] + cz + 1];
    baa = p[p[p[cx + 1] + cy    ] + cz    ];
    bba = p[p[p[cx + 1] + cy + 1] + cz    ];
    bab = p[p[p[cx + 1] + cy    ] + cz + 1];
    bbb = p[p[p[cx + 1] + cy + 1] + cz + 1];

    f32 x1 = lerp(grad(aaa, rx, ry,        rz), grad(baa, rx - 1.0f, ry,        rz), fx);
    f32 x2 = lerp(grad(aba, rx, ry - 1.0f, rz), grad(bba, rx - 1.0f, ry - 1.0f, rz), fx);
    f32 y1 = lerp(x1, x2, fy);

    f32 x3 = lerp(grad(aab, rx, ry,        rz - 1.0f), grad(bab, rx - 1.0f, ry,        rz - 1.0f), fx);
    f32 x4 = lerp(grad(abb, rx, ry - 1.0f, rz - 1.0f), grad(bbb, rx - 1.0f, ry - 1.0f, rz - 1.0f), fx);
    f32 y2 = lerp(x3, x4, fy);
    
    return (lerp(y1, y2, fz) + 1.0f) / 2.0f; 
}

f32 perlin_noise_3d_wrap(f32 x, f32 y, f32 z, i32 wrap) {
    i32 p[] = {
        151,160,137,91,90,15,
        131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
        190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
        88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
        77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
        102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
        135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
        5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
        223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
        129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
        251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
        49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
        138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
        151,160,137,91,90,15,
        131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
        190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
        88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
        77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
        102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
        135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
        5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
        223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
        129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
        251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
        49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
        138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
    };

    x = fmodf(x, wrap);
    y = fmodf(y, wrap);
    z = fmodf(z, wrap);

    i32 cx = (i32) floorf(x) % 255;
    i32 cy = (i32) floorf(y) % 255;
    i32 cz = (i32) floorf(z) % 255;

    f32 rx = x - floorf(x);
    f32 ry = y - floorf(y);
    f32 rz = z - floorf(z);

    f32 fx = fade(rx);
    f32 fy = fade(ry);
    f32 fz = fade(rz);

    i32 aaa, aba, aab, abb, baa, bba, bab, bbb;

    i32 cxw = (cx + 1) % wrap;
    i32 cyw = (cy + 1) % wrap;
    i32 czw = (cz + 1) % wrap;

    aaa = p[p[p[cx ] + cy ] + cz ];
    aba = p[p[p[cx ] + cyw] + cz ];
    aab = p[p[p[cx ] + cy ] + czw];
    abb = p[p[p[cx ] + cyw] + czw];
    baa = p[p[p[cxw] + cy ] + cz ];
    bba = p[p[p[cxw] + cyw] + cz ];
    bab = p[p[p[cxw] + cy ] + czw];
    bbb = p[p[p[cxw] + cyw] + czw];

    f32 x1 = lerp(grad(aaa, rx, ry,        rz), grad(baa, rx - 1.0f, ry,        rz), fx);
    f32 x2 = lerp(grad(aba, rx, ry - 1.0f, rz), grad(bba, rx - 1.0f, ry - 1.0f, rz), fx);
    f32 y1 = lerp(x1, x2, fy);

    f32 x3 = lerp(grad(aab, rx, ry,        rz - 1.0f), grad(bab, rx - 1.0f, ry,        rz - 1.0f), fx);
    f32 x4 = lerp(grad(abb, rx, ry - 1.0f, rz - 1.0f), grad(bbb, rx - 1.0f, ry - 1.0f, rz - 1.0f), fx);
    f32 y2 = lerp(x3, x4, fy);
    
    return (lerp(y1, y2, fz) + 1.0f) / 2.0f; 
}
