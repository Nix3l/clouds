#ifndef CLOUDS_RENDERER_H
#define CLOUDS_RENDERER_H

#include "base.h"
#include "clouds/clouds.h"
#include "mesh/mesh.h"
#include "framebuffer/fbo.h"

typedef struct {
    mesh_s mesh;
} cloud_renderer_s;

void init_cloud_renderer();

void render_cloud_volume(cloud_volume_s* volume, fbo_s* buffer);

#endif
