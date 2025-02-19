#ifndef CLOUDS_H
#define CLOUDS_H

#include "base.h"

#include "texture/texture.h"

typedef struct {
    v3f position; 
    v3f size;

    u32 resolution;

    // x -> low detail, y -> medium detail, z -> high detail
    v3i worley_cpa;
    
    f32 perlin_frequency;
    f32 perlin_amplitude;
    f32 perlin_lacunarity;
    f32 perlin_persistence;
    u32 perlin_octaves;

    f32 noise_persistence;

    texture_3d_s noise_texture;
} cloud_volume_s;

cloud_volume_s create_cloud_volume(u32 resolution, v3i worley_cpa);

#endif
