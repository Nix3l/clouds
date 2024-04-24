#include "pproc_renderer.h"

#include "game.h"

void init_pproc_renderer() {
    f32 vertices[] = {
         1.0f,  1.0f, 0.0f, // top right
         1.0f, -1.0f, 0.0f, // bottom right
        -1.0f, -1.0f, 0.0f, // bottom left
        -1.0f,  1.0f, 0.0f  // top left 
    };

    u32 indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    f32 uvs[] = {
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,
        0.0f, 1.0f
    };

    u32 width = game_state->window.width;
    u32 height = game_state->window.height;

    game_state->pproc_renderer = (pproc_renderer_s) {
        .back_buffer = create_fbo(width, height, 1),
        .screen_quad = create_mesh(vertices, uvs, NULL, NULL, indices, ARRAY_SIZE(indices), ARRAY_SIZE(vertices))
    };

    fbo_create_texture(
            &game_state->pproc_renderer.back_buffer,
            GL_COLOR_ATTACHMENT0,
            GL_RGB16F,
            GL_RGB);
}

void render_post_processing(fbo_s* target_buffer) {
    fbo_s* back_buffer = &game_state->pproc_renderer.back_buffer;
    mesh_s* quad = &game_state->pproc_renderer.screen_quad;

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

    glBindVertexArray(quad->vao);
    mesh_enable_attributes(quad);

    shader_start(&game_state->post_processing_shader);

    game_state->post_processing_shader.load_uniforms(NULL);

    glDrawElements(GL_TRIANGLES, quad->index_count, GL_UNSIGNED_INT, NULL);

    shader_stop();

    mesh_disable_attributes(&game_state->pproc_renderer.screen_quad);
    glBindVertexArray(0);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // copy the results of the post processing back to the target buffer
    fbo_copy_texture(back_buffer, target_buffer, GL_COLOR_ATTACHMENT0);
}
