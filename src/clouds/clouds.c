#include "clouds.h"

cloud_volume_s create_cloud_volume(i32 width, i32 height, i32 depth, u32 cells_per_axis) {
    return (cloud_volume_s) {
        .position = VECTOR_3_ZERO(),
        .scale = VECTOR_3_ONE(),

        .cells_per_axis = cells_per_axis,
        .noise_texture = create_texture_3d_format(width, height, depth, GL_R32F, GL_RED, NULL)
    };
}
