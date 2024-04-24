#include "skybox_renderer.h"

#include "game.h"

void init_skybox_renderer() {
    f32 vertices[8 * 3] = {
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f
    };
    
    u32 indices[12 * 3] = {
        // right
        1, 2, 6,
        6, 5, 1,

        // left
        0, 4, 7,
        7, 3, 0,

        // top
        4, 5, 6,
        6, 7, 4,

        // bottom
        0, 3, 2,
        2, 1, 0,

        // back
        0, 1, 5,
        5, 4, 0,

        // front
        3, 7, 6,
        6, 2, 3
    };

    game_state->skybox_renderer = (skybox_renderer_s) {
        .mesh = create_mesh(vertices, NULL, NULL, NULL, indices, ARRAY_SIZE(indices), ARRAY_SIZE(vertices))
    };

    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void render_skybox(texture_s* cubemap, fbo_s* framebuffer) {
    skybox_renderer_s* renderer = &game_state->skybox_renderer;

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->id);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);

    glViewport(0, 0, game_state->window.width, game_state->window.height);

    shader_start(&game_state->skybox_shader);

    game_state->skybox_shader.load_uniforms(NULL);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->id);

    glBindVertexArray(renderer->mesh.vao);
    mesh_enable_attributes(&renderer->mesh);

    glDrawElements(GL_TRIANGLES, renderer->mesh.index_count, GL_UNSIGNED_INT, NULL);

    mesh_disable_attributes(&renderer->mesh);
    glBindVertexArray(0);

    shader_stop();

    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glDisable(GL_DEPTH_TEST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
