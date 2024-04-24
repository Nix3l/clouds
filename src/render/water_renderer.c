#include "water_renderer.h"

#include "util/log.h"
#include "game.h"

void init_water_renderer() {
    game_state->water_renderer = (water_renderer_s) {
        .background_color = VECTOR_4(1.0f, 1.0f, 1.0f, 1.0f),
        .shader = &game_state->water_shader,
        .render_wireframe = false,
        .framebuffer = create_fbo(game_state->window.width, game_state->window.height, 1)
    };

    // no transparency for now
    fbo_create_texture(&game_state->water_renderer.framebuffer,
            GL_COLOR_ATTACHMENT0, 
            GL_RGB16F,
            GL_RGB);

    fbo_create_depth_texture(&game_state->water_renderer.framebuffer);
}

void render_water(entity_s* entity) {
    water_renderer_s* renderer = &game_state->water_renderer;
    fbo_s* framebuffer = &renderer->framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->id);

    glDrawBuffers(framebuffer->num_textures, framebuffer->attachments);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glViewport(0, 0, game_state->window.width, game_state->window.height);

    shader_start(&game_state->water_shader);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, game_state->skybox.id);

    game_state->water_shader.load_uniforms(entity);

    if(game_state->water_renderer.render_wireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glBindVertexArray(entity->mesh.vao);
    mesh_enable_attributes(&entity->mesh);

    glDrawElements(GL_TRIANGLES, entity->mesh.index_count, GL_UNSIGNED_INT, 0);

    mesh_disable_attributes(&entity->mesh);
    glBindVertexArray(0);

    if(game_state->water_renderer.render_wireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    shader_stop();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
