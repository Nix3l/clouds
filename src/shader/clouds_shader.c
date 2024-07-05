#include "clouds_shader.h"

#include "game.h"
#include "util/math.h"

static void bind_attributes() {
    shader_bind_attribute(&game_state->cloud_shader.program, 0, "vs_position");
}

static void load_uniforms(void* _data) {
    cloud_shader_s* shader = &game_state->cloud_shader;
    cloud_volume_s* volume = _data;

    shader_load_mat4(shader->u_projview, camera_projection_view(&game_state->camera));
    shader_load_mat4(shader->u_transformation, get_transformation_matrix(volume->position, VECTOR_3_ZERO(), volume->scale));
}

void init_cloud_shader() {
    shader_s shader = load_and_create_shader(
        "shader_cloud",
        "shader/cloud_vs.glsl",
        "shader/cloud_fs.glsl",
        bind_attributes,
        load_uniforms,
        &game_state->frame_arena
    );

    game_state->cloud_shader = (cloud_shader_s) {
        .program = shader,

        .u_projview = shader_get_uniform(&shader, "projview_mat"),
        .u_transformation = shader_get_uniform(&shader, "transformation_mat"),
    };
}
