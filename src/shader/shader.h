#ifndef SHADER_H
#define SHADER_H

#include "base.h"

typedef struct {
    // metadata
    char* name;
    char* vertex_full_path;
    char* fragment_full_path;

    GLuint program_id;

    // NOTE(nix3l): there might be no real reason to hold on to these after creation
    // see create_shader in shader.c for more info
    GLuint vertex_id;
    GLuint fragment_id;

    void (*load_uniforms) (void*);
} shader_s;

// takes in source code and compiles a shader accordingly
shader_s create_shader(
        char* name,
        char* vertex_src, char* fragment_src,
        void (*bind_attributes) (),
        void (*load_uniforms) (void*));

// takes in file paths on disk and compiles a shader accordingly
shader_s load_and_create_shader(
        char* name,
        char* vertex_path, char* fragment_path,
        void (*bind_attributes) (),
        void (*load_uniforms) (void*));

void destroy_shader(shader_s* shader);

void shader_start(shader_s* shader);
void shader_stop();

void shader_bind_attribute(shader_s* shader, GLuint attribute, char* attribute_name);

GLuint shader_get_uniform(shader_s* shader, char* uniform_name);
void shader_load_int(GLuint uniform, i32 value);
void shader_load_uint(GLuint uniform, u32 value);
void shader_load_float(GLuint uniform, f32 value);
void shader_load_bool(GLuint uniform, bool value);
void shader_load_vec2(GLuint uniform, vec2s value);
void shader_load_vec3(GLuint uniform, vec3s value);
void shader_load_mat4(GLuint uniform, mat4s value);

// TODO(nix3l): make wrapper functions for glUniform*v()

#endif
