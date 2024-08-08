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
                "shader/noise/noise.comp",
                (v3i) { .x = 16, .y = 16, .z = 16 },
                &game_state->frame_arena),
    };

    fbo_create_texture(&game_state->cloud_renderer.back_buffer, GL_COLOR_ATTACHMENT0, GL_RGB16F, GL_RGB);

    cloud_renderer_s* renderer = &game_state->cloud_renderer;
    compute_shader_s* shader = &game_state->cloud_renderer.noise_compute;

    renderer->u_resolution  = compute_shader_get_uniform(shader, "resolution");
    renderer->u_worley_cpa  = compute_shader_get_uniform(shader, "worley_cpa");
    renderer->u_persistence = compute_shader_get_uniform(shader, "persistence");

    renderer->u_volume = compute_shader_get_uniform(shader, "volume");
}

static void generate_volume_perlin_noise(f32* data, cloud_volume_s* volume) {
    f32* curr = data;

    for(u32 z = 0; z < volume->resolution; z ++) {
        for(u32 y = 0; y < volume->resolution; y ++) {
            for(u32 x = 0; x < volume->resolution; x ++) {
                v3f input = V3F((f32)x, (f32)y, (f32)z);
                input = glms_vec3_scale(input, volume->perlin_frequency / volume->resolution);

                f32 res = 0.0f;
                f32 amp = volume->perlin_amplitude;
                f32 max_val = 0.0f;
                for(usize i = 0; i < volume->perlin_octaves; i ++) {
                    res += amp * perlin_noise_3d_wrap(input.x, input.y, input.z, 16);

                    max_val += amp;

                    input = glms_vec3_scale(input, volume->perlin_lacunarity);
                    amp *= volume->perlin_persistence;
                }

                *(curr++) = (res / max_val);
            }
        }
    }
}

static void generate_volume_points_for_lod(f32* data, u32 cpa) {
    // generate the point offsets
    // points are represented as offsets from the bottom left corner of a cell
    // and each cell is given a work group
    f32 cell_size = 1.0f / cpa;

    f32* curr_point = data;
    // TODO(nix3l): seed
    for(u32 z = 0; z < cpa; z ++) {
        for(u32 y = 0; y < cpa; y ++) {
            for(u32 x = 0; x < cpa; x ++) {
                *(curr_point++) = RAND_IN_RANGE(0.0f, cell_size);
                *(curr_point++) = RAND_IN_RANGE(0.0f, cell_size);
                *(curr_point++) = RAND_IN_RANGE(0.0f, cell_size);
                *(curr_point++) = 0.0f; // padding for the SSBO to align to 16 bytes
            }
        }
    }
}

void render_cloud_noise(cloud_volume_s* volume, arena_s* arena) {
    u32 ld_points = volume->worley_cpa.x * volume->worley_cpa.x * volume->worley_cpa.x;
    u32 md_points = volume->worley_cpa.y * volume->worley_cpa.y * volume->worley_cpa.y;
    u32 hd_points = volume->worley_cpa.z * volume->worley_cpa.z * volume->worley_cpa.z;
    u32 total_points = ld_points + md_points + hd_points;
    u32 total_voxels = volume->resolution * volume->resolution * volume->resolution;

    f32* point_data = arena_push(arena, 4 * sizeof(f32) * total_points);
    
    // generate sets of point data for each level of detail of worley noise
    generate_volume_points_for_lod(point_data, volume->worley_cpa.x);
    generate_volume_points_for_lod(point_data + ld_points, volume->worley_cpa.y);
    generate_volume_points_for_lod(point_data + ld_points + md_points, volume->worley_cpa.z);

    f32* perlin_data = arena_push(arena, sizeof(f32) * total_voxels);
    
    // generate perlin noise
    generate_volume_perlin_noise(perlin_data, volume);

    cloud_renderer_s* renderer = &game_state->cloud_renderer; 
    compute_shader_s* shader = &renderer->noise_compute;

    compute_shader_start(shader);

    // NOTE(nix3l): not sure if its smart to generate a storage buffer
    // every time we generate the noise but i dont exactly see a reason
    // why i should hold on to one instead
    GLuint worley_buffer;
    glGenBuffers(1, &worley_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, worley_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 4 * sizeof(GLfloat) * total_points, point_data, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, worley_buffer);

    GLuint perlin_buffer;
    glGenBuffers(1, &perlin_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, perlin_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat) * total_voxels, perlin_data, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, perlin_buffer);

    shader_load_int(renderer->u_volume, 2);
    glBindImageTexture(2, volume->noise_texture.id, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    shader_load_int(renderer->u_resolution, volume->resolution);
    shader_load_ivec3(renderer->u_worley_cpa, volume->worley_cpa);
    shader_load_float(renderer->u_persistence, volume->noise_persistence);

    compute_shader_dispatch_groups(volume->resolution, volume->resolution, volume->resolution);
    compute_shader_stop();

    // delete the buffer as we dont need it anymore
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glDeleteBuffers(1, &worley_buffer);
    glDeleteBuffers(1, &perlin_buffer);
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
