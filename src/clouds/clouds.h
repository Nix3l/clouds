#ifndef CLOUDS_H
#define CLOUDS_H

#include "base.h"

#include "texture/texture.h"

typedef struct {
    v3f position; 
    v3f size;

    u32 resolution;

    // x -> low detail, y -> medium detail, z -> high detail
    v3i cells_per_axis;

    texture_3d_s noise_texture;
} cloud_volume_s;

cloud_volume_s create_cloud_volume(u32 resolution, v3i cells_per_axis);

#endif
