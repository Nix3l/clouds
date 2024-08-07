#include "clouds.h"

cloud_volume_s create_cloud_volume(u32 resolution, v3i worley_cpa) {
    return (cloud_volume_s) {
        .position = V3F_ZERO(),
        .size = V3F_ONE(),

        .resolution = resolution,
        .worley_cpa = worley_cpa,

        .perlin_frequency   = 4.0f,
        .perlin_lacunarity  = 0.0f,
        .perlin_amplitude   = 1.0f,
        .perlin_persistence = 0.0f,
        .perlin_octaves     = 1.0f,

        .noise_persistence  = 0.625f,

        .noise_texture = create_texture_3d_format(resolution, resolution, resolution, GL_RGBA32F, GL_RGBA, NULL)
    };
}
