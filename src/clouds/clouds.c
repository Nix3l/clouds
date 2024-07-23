#include "clouds.h"

cloud_volume_s create_cloud_volume(u32 resolution, v3i worley_cpa) {
    return (cloud_volume_s) {
        .position = VECTOR_3_ZERO(),
        .size = VECTOR_3_ONE(),

        .resolution = resolution,
        .worley_cpa = worley_cpa,

        // TODO(nix3l): perlin parameters

        .noise_texture = create_texture_3d_format(resolution, resolution, resolution, GL_RGBA32F, GL_RGBA, NULL)
    };
}
