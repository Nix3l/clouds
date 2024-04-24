#ifndef POST_PROCESSING_SHADER_H
#define POST_PROCESSING_SHADER_H

#include "base.h"

typedef struct {
    GLuint scene;
    GLuint depth;
    GLuint near_plane;
    GLuint far_plane;

    GLuint fog_distance;
    GLuint fog_ramp;
    GLuint fog_attenuation;
    GLuint fog_color;

    GLuint sun_attenuation;
    GLuint halo_intensity;
    GLuint sun_intensity;
    GLuint sun_color;
    GLuint sun_dir;

    GLuint projection;
    GLuint view;
} pproc_shader_uniforms_s;

void init_post_processing_shaders();

#endif
