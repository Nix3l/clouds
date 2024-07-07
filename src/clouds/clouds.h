#ifndef CLOUDS_H
#define CLOUDS_H

#include "base.h"

#include "texture/texture.h"

typedef struct {
    v3f position; 
    v3f scale;

    u32 cells_per_axis;

    texture_3d_s noise_texture;
} cloud_volume_s;

cloud_volume_s create_cloud_volume(i32 noise_width, i32 noise_height, i32 noise_depth, u32 cells_per_axis);

#endif
