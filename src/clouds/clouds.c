#include "clouds.h"

cloud_volume_s create_cloud_volume(u32 resolution, u32 cells_per_axis) {
    return (cloud_volume_s) {
        .position = VECTOR_3_ZERO(),
        .size = VECTOR_3_ONE(),

        .resolution = resolution,
        .cells_per_axis = cells_per_axis,

        .noise_texture = create_texture_3d_format(resolution, resolution, resolution, GL_R32F, GL_RED, NULL)
    };
}
