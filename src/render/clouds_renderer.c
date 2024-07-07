#include "clouds_renderer.h"

#include "game.h"

void init_cloud_renderer() {
    game_state->cloud_renderer = (cloud_renderer_s) {
        // .mesh = primitive_cube_mesh(),
        .mesh = load_mesh_from_file("cube.glb", &game_state->mesh_arena),
        .noise_compute = load_and_create_compute_shader(
                "shader/noise/worley.comp",
                (v3i) { .x = 128, .y = 128, .z = 128 },
                &game_state->frame_arena),
    };

    cloud_renderer_s* renderer = &game_state->cloud_renderer;
    compute_shader_s* shader = &game_state->cloud_renderer.noise_compute;

    renderer->u_cells_per_axis = compute_shader_get_uniform(shader, "cells_per_axis");
}

void render_cloud_noise(cloud_volume_s* volume, arena_s* arena) {
    u32 width = volume->noise_texture.width, height = volume->noise_texture.height, depth = volume->noise_texture.depth;

    u32 cell_width  = width  / volume->cells_per_axis;
    u32 cell_height = height / volume->cells_per_axis;
    u32 cell_depth  = depth  / volume->cells_per_axis;

    u32 total_points = volume->cells_per_axis * volume->cells_per_axis * volume->cells_per_axis;
    f32* point_data = arena_push(arena, 3 * sizeof(f32) * total_points);
    f32* curr_point = point_data;

    for(u32 z = 0; z < volume->cells_per_axis; z ++) {
        for(u32 y = 0; y < volume->cells_per_axis; y ++) {
            for(u32 x = 0; x < volume->cells_per_axis; x ++) {
                *(curr_point++) = cell_width  * x + RAND_IN_RANGE(0.0f, cell_width);
                *(curr_point++) = cell_height * y + RAND_IN_RANGE(0.0f, cell_height);
                *(curr_point++) = cell_depth  * z + RAND_IN_RANGE(0.0f, cell_depth);
            }
        }
    }

    cloud_renderer_s* renderer = &game_state->cloud_renderer; 
    compute_shader_s* shader = &renderer->noise_compute;

    compute_shader_start(shader);

    // NOTE(nix3l): not sure if its smart to generate a storage buffer
    // every time we generate the noise but i dont exactly see a reason
    // why i should hold on to one instead
    GLuint storage_buffer;
    glGenBuffers(1, &storage_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, storage_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 3 * sizeof(f32) * total_points, point_data, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, storage_buffer);

    glBindImageTexture(0, volume->noise_texture.id, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32F);

    shader_load_int(renderer->u_cells_per_axis, volume->cells_per_axis);

    compute_shader_dispatch(shader);
    compute_shader_stop();

    // delete the buffer as we dont need it anymore
    glDeleteBuffers(1, &storage_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void render_cloud_volume(cloud_volume_s* volume, fbo_s* buffer) {
    cloud_shader_s* shader = &game_state->cloud_shader;
    mesh_s* cube = &game_state->cloud_renderer.mesh;

    glEnable(GL_DEPTH_TEST);

    // glBindFramebuffer(GL_FRAMEBUFFER, buffer->id);
    // glDrawBuffer(GL_COLOR_ATTACHMENT0);

    glBindVertexArray(cube->vao);
    mesh_enable_attributes(cube);
    
    shader_start(&shader->program);

    shader->program.load_uniforms(volume);

    glDrawElements(GL_TRIANGLES, cube->index_count, GL_UNSIGNED_INT, NULL);

    shader_stop();

    mesh_disable_attributes(cube);
    glBindVertexArray(0);

    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
