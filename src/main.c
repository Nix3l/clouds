#define STB_IMAGE_IMPLEMENTATION

// NOTE(nix3l): https://umu.diva-portal.org/smash/get/diva2:1223894/FULLTEXT01.pdf
//              ^^ source for most of this

// TODO(nix3l): figure out volumetric rendering

#include "game.h"
#include "util/log.h"
#include "util/math.h"

// TODO(nix3l): this is a slightly older version of cimgui
// since i completely forget how i compiled the backends the first time around lol
// at some point probably try to compile the new backends. the new version of the main lib is still here though

#include "platform/platform.h"
#include "params/params.h"

game_memory_s* game_memory = NULL;
game_state_s* game_state = NULL;

static void show_debug_stats_window() {
    if(is_key_pressed(GLFW_KEY_F1)) game_state->show_debug_stats_window = !game_state->show_debug_stats_window;
    if(!game_state->show_debug_stats_window) return;
    igBegin("stats", &game_state->show_debug_stats_window, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    // FRAME STATS
    igSeparator();
    igText("elapsed time: %lf\n", game_state->curr_time);
    igText("delta time: %f\n", game_state->delta_time);
    igText("frame count: %u\n", game_state->frame_count);
    igText("fps: %u\n", game_state->fps);

    igSeparator();

    // MEMORY
    igText("total transient memory: %u\n", game_memory->transient_storage_size);
    igText("total permenant memory: %u\n", game_memory->permenant_storage_size);

    igText("permenant memory in use: %u/%u\n",
            sizeof(game_state_s) + 
            game_state->shader_arena.size + 
            game_state->mesh_arena.size +
            game_state->fbo_arena.size +
            game_state->params_arena.size +
            game_state->texture_arena.size +
            game_memory->permenant_storage_size);
    igIndent(12.0f);
    igText("of which state: %u\n", sizeof(game_state_s));
    igText("of which shaders: %u/%u\n", game_state->shader_arena.size, game_state->shader_arena.capacity);
    igText("of which framebuffers: %u/%u\n", game_state->fbo_arena.size, game_state->fbo_arena.capacity);
    igText("of which parameters: %u/%u\n", game_state->params_arena.size, game_state->params_arena.capacity);
    igText("of which textures: %u/%u\n", game_state->texture_arena.size, game_state->texture_arena.capacity);
    igText("of which meshes: %u/%u\n", game_state->mesh_arena.size, game_state->mesh_arena.capacity);
    igText("of which frame: %u/%u\n", game_state->frame_arena.size, game_state->frame_arena.capacity);
    igUnindent(12.0f);

    igEnd();
}

static void show_settings_window() {
    if(is_key_pressed(GLFW_KEY_F2)) game_state->show_settings_window = !game_state->show_settings_window;
    if(!game_state->show_settings_window) return;

    igBegin("settings", &game_state->show_settings_window, ImGuiWindowFlags_None);

    // SETTINGS

    if(igCollapsingHeader_TreeNodeFlags("camera", ImGuiTreeNodeFlags_None)) {
        igPushID_Str("camera_settings");
        igDragFloat("sensetivity", &game_state->camera.sens, 10.0f, 0.0f, MAX_f32, "%.0f", ImGuiSliderFlags_None);
        igDragFloat("move speed", &game_state->camera.speed, 1.0f, 0.0f, MAX_f32, "%.0f", ImGuiSliderFlags_None);

        igDragFloat3("position", game_state->camera.position.raw, 1.0f, -MAX_f32, MAX_f32, "%.1f", ImGuiSliderFlags_ReadOnly);
        igDragFloat3("rotation", game_state->camera.rotation.raw, 1.0f, -MAX_f32, MAX_f32, "%.1f", ImGuiSliderFlags_ReadOnly);
        igPopID();
    }

    if(igCollapsingHeader_TreeNodeFlags("volume", ImGuiTreeNodeFlags_None)) {
        igPushID_Str("volume_settings");
        cloud_volume_s* volume = &game_state->volume;

        igDragFloat3("position", volume->position.raw, 1.0f, -MAX_f32, MAX_f32, "%.1f", ImGuiSliderFlags_None);
        igDragFloat3("size", volume->size.raw, 1.0f, 0.0f, MAX_f32, "%.1f", ImGuiSliderFlags_None);

        igSeparator();

        if(igButton("render noise", (ImVec2) { .x = -0.1f, .y = 24.0f }))
            render_cloud_noise(volume, &game_state->frame_arena);

        igDragInt3("worly cpa", volume->worley_cpa.raw, 0.4f, 1, volume->resolution, "%d", ImGuiSliderFlags_None);

        igDragFloat("perlin frequency", &volume->perlin_frequency, 0.1f, 0.0f, MAX_f32, "%.2f", ImGuiSliderFlags_None);
        igDragFloat("perlin amplitude", &volume->perlin_amplitude, 0.1f, 0.0f, MAX_f32, "%.2f", ImGuiSliderFlags_None);
        igDragFloat("perlin lacunarity", &volume->perlin_lacunarity, 0.1f, 0.0f, MAX_f32, "%.2f", ImGuiSliderFlags_None);
        igDragFloat("perlin persistence", &volume->perlin_persistence, 0.1f, 0.0f, MAX_f32, "%.2f", ImGuiSliderFlags_None);

        // why must you do this to me imgui
        u32 min = 1, max = MAX_u32;
        igDragScalar("perlin octaves", ImGuiDataType_U32, &volume->perlin_octaves, 0.1f, &min, &max, "%u", ImGuiSliderFlags_None);

        igDragFloat("noise persistence", &volume->noise_persistence, 0.1f, 0.0f, MAX_f32, "%.2f", ImGuiSliderFlags_None);

        igPopID();
    }

    if(igCollapsingHeader_TreeNodeFlags("clouds", ImGuiTreeNodeFlags_None)) {
        igPushID_Str("cloud_shader_settings");
        cloud_shader_s* shader = &game_state->cloud_shader;

        u32 min = 1, max = MAX_u32;
        igDragScalar("cloud raymarch steps", ImGuiDataType_U32, &shader->cloud_march_steps, 0.1f, &min, &max, "%u", ImGuiSliderFlags_None);

        igDragFloat("absorption", &shader->absorption, 0.05f, 0.0f, MAX_f32, "%.2f", ImGuiSliderFlags_None);

        igPopID();
    }
  
    igEnd();
}

// NOTE(nix3l): this is a very rudimentary version of time profiling
// later on i should implement a stack based thing where i can push and pop
// before/after actions to more accurately profile, but that goes beyond
// the scope of this project
static void update_frame_stats() {
    game_state->old_time = game_state->curr_time;
    game_state->curr_time = glfwGetTime() * game_state->time_scale;
    game_state->delta_time = game_state->curr_time - game_state->old_time;
    game_state->frame_count ++;
    game_state->fps_counter ++;
    game_state->fps_timer += game_state->delta_time;

    // update fps every 0.16s or so
    // makes it flicker less than updating it every frame
    if(game_state->fps_timer >= (1/6.0f) * game_state->time_scale) {
        game_state->fps = game_state->fps_counter / game_state->fps_timer;
        game_state->fps_counter = 0;
        game_state->fps_timer = 0.0f;
    }
}

static arena_s partition_permenant_memory(void** mem, usize size, usize* remaining) {
    ASSERT(size <= *remaining);
    arena_s arena = arena_create_in_block(*mem, size);

    *remaining -= size;
    *mem += size;
    return arena;
}

static void init_game_state(usize permenant_memory_to_allocate, usize transient_memory_to_allocate) {
    ASSERT(sizeof(game_state_s) < permenant_memory_to_allocate);

    // INITIALISE MEMORY
    game_memory = mem_alloc(sizeof(game_memory_s));
    ASSERT(game_memory);

    game_memory->permenant_storage_size = permenant_memory_to_allocate;
    game_memory->permenant_storage = mem_alloc(permenant_memory_to_allocate);
    ASSERT(game_memory->permenant_storage);
    MEM_ZERO(game_memory->permenant_storage, game_memory->permenant_storage_size);
    
    game_memory->transient_storage_size = transient_memory_to_allocate;
    game_memory->transient_storage = mem_alloc(transient_memory_to_allocate);
    ASSERT(game_memory->transient_storage);
    MEM_ZERO(game_memory->transient_storage, game_memory->transient_storage_size);

    // PARTITIONING MEMORY
    game_state = game_memory->permenant_storage;

    void* memory = game_memory->permenant_storage + sizeof(game_state_s);
    usize remaining_memory = permenant_memory_to_allocate - sizeof(game_state_s);

    game_state->shader_arena  = partition_permenant_memory(&memory, KILOBYTES(1),     &remaining_memory);
    game_state->fbo_arena     = partition_permenant_memory(&memory, KILOBYTES(1),     &remaining_memory);
    game_state->params_arena  = partition_permenant_memory(&memory, KILOBYTES(4),     &remaining_memory);
    game_state->texture_arena = partition_permenant_memory(&memory, KILOBYTES(4),     &remaining_memory);
    game_state->mesh_arena    = partition_permenant_memory(&memory, KILOBYTES(8),     &remaining_memory);
    game_state->frame_arena   = partition_permenant_memory(&memory, remaining_memory, &remaining_memory);

    // IO
    create_window(1600, 900, "clouds");
    init_input();

    // SHADERS
    init_cloud_shader();

    // RENDERER
    game_state->camera = (camera_s) {
        .position   = V3F(0.0f, 0.0f, 0.0f),
        .rotation   = V3F(0.0f, 0.0f, 0.0f),
        
        .fov        = 70.0f,
        .near_plane = 0.001f,
        .far_plane  = 1500.0f,

        .speed      = 100.0f,
        .sens       = 7500.0f
    };

    game_state->sun = (directional_light_s) {
        .color = V3F_RGB(239.0f, 227.0f, 200.0f),
        .direction = V3F(-0.1f, -0.3f, 0.04f),
        .intensity = 0.9f
    };

    game_state->screen_buffer = create_fbo(
            game_state->window.width,
            game_state->window.height,
            1,
            &game_state->fbo_arena);

    fbo_create_texture(&game_state->screen_buffer, GL_COLOR_ATTACHMENT0, GL_RGB16F, GL_RGB);
    fbo_create_depth_texture(&game_state->screen_buffer);

    init_pproc_renderer();
    init_cloud_renderer();

    // VOLUMES
    game_state->volume = create_cloud_volume(128, (v3i) { .x = 8, .y = 16, .z = 32 });
    game_state->volume.position = V3F(0.0f, -80.0f, -350.0f);
    game_state->volume.size = V3F(128.0f, 64.0f, 128.0f);

    game_state->volume.perlin_frequency   = 3.0f;
    game_state->volume.perlin_lacunarity  = 2.0f;
    game_state->volume.perlin_amplitude   = 0.5f;
    game_state->volume.perlin_persistence = 0.5f;
    game_state->volume.perlin_octaves     = 6;
    game_state->volume.noise_persistence  = 0.4f;

    game_state->cloud_shader.cloud_march_steps = 8;
    game_state->cloud_shader.absorption = 0.5f;

    // GUI
    init_imgui();
    game_state->time_scale = 1.0f;

    glfwSetInputMode(game_state->window.glfw_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    // LOAD PARAMETERS

    // OTHER

    render_cloud_noise(&game_state->volume, &game_state->frame_arena);
}

static void terminate_game() {
    shutdown_imgui();
    destroy_window();

    mem_free(game_memory->transient_storage);
    mem_free(game_memory->permenant_storage);
    mem_free(game_memory);
}

int main(void) {
    init_game_state(GIGABYTES(1), MEGABYTES(16));

    while(!glfwWindowShouldClose(game_state->window.glfw_window)) {
        // UPDATE
        update_frame_stats();

        update_camera(&game_state->camera);

        // RENDER
        fbo_clear(&game_state->screen_buffer, V3F(0.098f, 0.11f, 0.11f), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        render_cloud_volume(&game_state->volume, &game_state->screen_buffer);

        fbo_copy_texture_to_screen(&game_state->screen_buffer, GL_COLOR_ATTACHMENT0);

        update_imgui();
        show_debug_stats_window();
        show_settings_window();
        render_imgui();

        arena_clear(&game_state->frame_arena);

        glfwSwapBuffers(game_state->window.glfw_window);

        update_input();
        glfwPollEvents();
    }

    terminate_game();

    return 0;
}
