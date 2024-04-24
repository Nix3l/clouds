#ifndef SKYBOX_SHADER_H
#define SKYBOX_SHADER_H

#include "base.h"
#include "shader/shader.h"

// TODO(nix3l)

typedef struct {
    GLuint projection_view;
    GLuint cubemap;
} skybox_shader_uniforms_s;

void init_skybox_shader();

#endif
