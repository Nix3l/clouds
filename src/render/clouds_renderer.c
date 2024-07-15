#include "clouds_renderer.h"

#include "game.h"

void init_cloud_renderer() {
    game_state->cloud_renderer = (cloud_renderer_s) {
        .back_buffer = create_fbo(
                game_state->window.width,
                game_state->window.height,
                1,
                &game_state->fbo_arena),
        .noise_compute = load_and_create_compute_shader(
                "shader/noise/worley.comp",
                (v3i) { .x = 16, .y = 16, .z = 16 },
                &game_state->frame_arena),
    };

    fbo_create_texture(&game_state->cloud_renderer.back_buffer, GL_COLOR_ATTACHMENT0, GL_RGB16F, GL_RGB);

    cloud_renderer_s* renderer = &game_state->cloud_renderer;
    compute_shader_s* shader = &game_state->cloud_renderer.noise_compute;

    renderer->u_resolution = compute_shader_get_uniform(shader, "resolution");
    renderer->u_cells_per_axis = compute_shader_get_uniform(shader, "cells_per_axis");

    renderer->u_volume = compute_shader_get_uniform(shader, "volume");
}

void render_cloud_noise(cloud_volume_s* volume, arena_s* arena) {
    // generate the point offsets
    // points are represented as offsets from the bottom left corner of a cell
    // and each cell is given a work group
    f32 cell_size = 1.0f / volume->cells_per_axis;

    u32 total_points = volume->cells_per_axis * volume->cells_per_axis * volume->cells_per_axis;
    f32* point_data = arena_push(arena, 3 * sizeof(f32) * total_points);
    f32* curr_point = point_data;

    // TODO(nix3l): seed

    for(u32 z = 0; z < volume->cells_per_axis; z ++) {
        for(u32 y = 0; y < volume->cells_per_axis; y ++) {
            for(u32 x = 0; x < volume->cells_per_axis; x ++) {
                *(curr_point++) = RAND_IN_RANGE(0.0f, cell_size);
                *(curr_point++) = RAND_IN_RANGE(0.0f, cell_size);
                *(curr_point++) = RAND_IN_RANGE(0.0f, cell_size);
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

    shader_load_int(renderer->u_volume, 1);
    glBindImageTexture(1, volume->noise_texture.id, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32F);

    shader_load_float(renderer->u_resolution, (float) volume->resolution);
    shader_load_int(renderer->u_cells_per_axis, volume->cells_per_axis);

    compute_shader_dispatch_groups(volume->resolution, volume->resolution, volume->resolution);
    compute_shader_stop();

    // delete the buffer as we dont need it anymore
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glDeleteBuffers(1, &storage_buffer);
}

void render_cloud_volume(cloud_volume_s* volume, fbo_s* target_buffer) {
    cloud_shader_s* shader = &game_state->cloud_shader;
    fbo_s* back_buffer = &game_state->cloud_renderer.back_buffer;
    mesh_s* quad = &game_state->screen_quad;

    glDisable(GL_DEPTH_TEST);

    glBindFramebuffer(GL_FRAMEBUFFER, back_buffer->id);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, target_buffer->id);

    // color
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, target_buffer->textures[0].id);

    // depth
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, target_buffer->depth.id);

    // noise 
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_3D, volume->noise_texture.id);

    glBindVertexArray(quad->vao);
    mesh_enable_attributes(quad);

    shader_start(&shader->program);

    shader->program.load_uniforms(volume);

    glDrawElements(GL_TRIANGLES, quad->index_count, GL_UNSIGNED_INT, NULL);

    shader_stop();

    mesh_disable_attributes(quad);
    glBindVertexArray(0);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // TODO(nix3l): figure out what to do about the depth buffer here
    fbo_copy_texture(back_buffer, target_buffer, GL_COLOR_ATTACHMENT0);
}
