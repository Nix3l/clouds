#ifndef CLOUDS_SHADER_H
#define CLOUDS_SHADER_H

#include "shader.h"

typedef struct {
    shader_s program;

    uniform_t u_projview;
    uniform_t u_transformation;
} cloud_shader_s;

void init_cloud_shader();

#endif
