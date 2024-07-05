#include "clouds_renderer.h"

#include "game.h"

void init_cloud_renderer() {
    game_state->cloud_renderer = (cloud_renderer_s) {
        // .mesh = primitive_cube_mesh(),
        .mesh = load_mesh_from_file("cube.glb", &game_state->mesh_arena)
    };
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
