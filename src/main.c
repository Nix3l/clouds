#define STB_IMAGE_IMPLEMENTATION

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
            game_state->texture_arena.size,
            game_memory->permenant_storage_size);
    igIndent(12.0f);
    igText("of which state: %u\n", sizeof(game_state_s));
    igText("of which shaders: %u/%u\n", game_state->shader_arena.size, game_state->shader_arena.capacity);
    igText("of which framebuffers: %u/%u\n", game_state->fbo_arena.size, game_state->fbo_arena.capacity);
    igText("of which parameters: %u/%u\n", game_state->params_arena.size, game_state->params_arena.capacity);
    igText("of which textures: %u/%u\n", game_state->texture_arena.size, game_state->texture_arena.capacity);
    igText("of which meshes: %u/%u\n", game_state->mesh_arena.size, game_state->mesh_arena.capacity);
    igUnindent(12.0f);

    igEnd();
}

static void show_settings_window() {
    if(is_key_pressed(GLFW_KEY_F2)) game_state->show_settings_window = !game_state->show_settings_window;
    if(!game_state->show_settings_window) return;

    igBegin("settings", &game_state->show_settings_window, ImGuiWindowFlags_None);

    // CAMERA
    igDragFloat("move speed", &game_state->camera.speed, 1.0f, 0.0f, MAX_f32, "%.2f", ImGuiSliderFlags_None);
    igDragFloat("sensetivity", &game_state->camera.sens, 10.0f, 0.0f, MAX_f32, "%.2f", ImGuiSliderFlags_None);

    // WATER TRANSFORM
    igDragFloat3("scale", game_state->water_entity.transform.scale.raw, 0.1f, -MAX_f32, MAX_f32, "%.3f", ImGuiSliderFlags_None);;

    igSeparator();

    // RENDERER
    if(igButton("render wireframe", (ImVec2) { .x = -1.0f, .y = 24.0f }))
        game_state->water_renderer.render_wireframe = !game_state->water_renderer.render_wireframe;

    // TIME SCALE
    igSeparator();
    igDragFloat("time scale", &game_state->time_scale, 0.1f, 0.01f, MAX_f32, "%.2f", ImGuiSliderFlags_None);
    igSeparator();

    // PARAMS FILE
    igInputText("parameters filepath", game_state->params_filepath, sizeof(game_state->params_filepath), ImGuiInputTextFlags_None, NULL, NULL);

    if(igButton("load params", (ImVec2) { .x = igGetWindowWidth()/2.0f-8.0f, .y = 24.0f }))
        load_parameters_from_file(game_state->params_filepath, &game_state->params_arena);

    igSameLine(igGetWindowWidth()/2.0f, 4.0f);

    if(igButton("write params", (ImVec2) { .x = igGetWindowWidth()/2.0f, .y = 24.0f }))
        write_parameters_to_file(game_state->params_filepath, &game_state->params_arena);

    igSeparator();

    // SHADER VARIABLES
    void* temp_mem = game_memory->transient_storage;
    MEM_ZERO(temp_mem, 8);
    const u32 uzero = 0;
    const u32 umax = MAX_u32;
    if(igCollapsingHeader_TreeNodeFlags("waves", ImGuiTreeNodeFlags_None)) {
        igDragScalar("seed", ImGuiDataType_U32, &game_state->seed, 0.1f, &uzero, &umax, "%u", ImGuiSliderFlags_None);
        igDragFloat("speed ramp", &game_state->speed_ramp, 0.01f, -MAX_f32, MAX_f32, "%.3f", ImGuiSliderFlags_None);
        igDragFloat("angle seed", &game_state->angle_seed, 1.0f, -MAX_f32, MAX_f32, "%.3f", ImGuiSliderFlags_None);
        igDragFloat("angle offset", &game_state->angle_offset, 1.0f, 0.0f, 360.0f, "%.3f", ImGuiSliderFlags_None);
        igDragFloat("vertex drag", &game_state->vertex_drag, 1.0f, 0.0f, 360.0f, "%.3f", ImGuiSliderFlags_None);
        igSeparator();

        for(u32 i = 0; i < TOTAL_WAVES; i ++) {
            wave_s* wave = &game_state->waves[i];
            char* label = temp_mem;
            snprintf(label, 8, "wave %u", i);
            
            if(igTreeNode_Str(label)) {
                igDragScalar("iterations", ImGuiDataType_U32, &wave->iterations, 0.1f, &uzero, &umax, "%u", ImGuiSliderFlags_None);
                igDragFloat("wavelength", &wave->wavelength, 0.01f, 0.0f, MAX_f32, "%.3f", ImGuiSliderFlags_None);
                igDragFloat("amplitude", &wave->amplitude, 0.01f, 0.0f, MAX_f32, "%.3f", ImGuiSliderFlags_None);
                igDragFloat("steepness", &wave->steepness, 0.01f, 0.0f, MAX_f32, "%.3f", ImGuiSliderFlags_None);
                igDragFloat("speed", &wave->speed, 0.01f, 0.0f, MAX_f32, "%.3f", ImGuiSliderFlags_None);
                igDragFloat2("direction", wave->direction.raw, 0.01f, -1.0f, 1.0f, "%.3f", ImGuiSliderFlags_None);
                igDragFloat("wavelength factor", &wave->w_factor, 0.01f, 0.0f, MAX_f32, "%.3f", ImGuiSliderFlags_None);
                igDragFloat("amplitude factor", &wave->a_factor, 0.01f, 0.0f, MAX_f32, "%.3f", ImGuiSliderFlags_None);
                igTreePop();
            }
        }
    }

    igSeparator();

    igDragFloat("normal bias", &game_state->normal_bias, 0.01f, 0.0f, MAX_f32, "%.3f", ImGuiSliderFlags_None);

    igDragFloat3("light direction", game_state->sun.direction.raw, 0.1f, -1.0f, 1.0f, "%.3f", ImGuiSliderFlags_None);
    igColorEdit3("light color", game_state->sun.color.raw, ImGuiColorEditFlags_None);
    igDragFloat("light intensity", &game_state->sun.intensity, 0.01f, 0.0f, MAX_f32, "%.3f", ImGuiSliderFlags_None);

    igColorEdit3("water color", game_state->water_color.raw, ImGuiColorEditFlags_None);

    igColorEdit3("tip color", game_state->tip_color.raw, ImGuiColorEditFlags_None);
    igDragFloat("tip attenuation", &game_state->tip_attenuation, 0.10f, 0.0f, MAX_f32, "%.3f", ImGuiSliderFlags_None);

    igDragFloat("specular factor", &game_state->specular_factor, 0.10f, 0.0f, MAX_f32, "%.3f", ImGuiSliderFlags_None);
    igDragFloat("specular strength", &game_state->specular_strength, 0.10f, 0.0f, MAX_f32, "%.3f", ImGuiSliderFlags_None);
    igDragFloat("reflectiveness", &game_state->r0, 0.10f, 0.0f, MAX_f32, "%.3f", ImGuiSliderFlags_None);
    
    igDragFloat("ambient", &game_state->ambient, 0.01f, 0.0f, MAX_f32, "%.3f", ImGuiSliderFlags_None);
    igColorEdit3("ambient color", game_state->ambient_color.raw, ImGuiColorEditFlags_None);

    igDragFloat("scatter amount", &game_state->scatter_amount, 0.01f, 0.0f, MAX_f32, "%.3f", ImGuiSliderFlags_None);
    igDragFloat("scatter angle", &game_state->scatter_angle, 0.01f, 0.0f, MAX_f32, "%.3f", ImGuiSliderFlags_None);
    igColorEdit3("scatter color", game_state->scatter_color.raw, ImGuiColorEditFlags_None);

    igDragFloat("environment normal bias", &game_state->env_normal_bias, 0.01f, 0.0f, MAX_f32, "%.3f", ImGuiSliderFlags_None);
    igDragFloat("environment reflection strength", &game_state->reflection_strength, 0.01f, 0.0f, MAX_f32, "%.3f", ImGuiSliderFlags_None);

    igSeparator();

    igDragFloat("fog distance", &game_state->fog_distance, 0.01f, 0.0f, MAX_f32, "%.3f", ImGuiSliderFlags_None);
    igDragFloat("fog ramp", &game_state->fog_ramp, 0.01f, 0.0f, MAX_f32, "%.3f", ImGuiSliderFlags_None);
    igDragFloat("fog attenuation", &game_state->fog_attenuation, 0.01f, 0.0f, MAX_f32, "%.3f", ImGuiSliderFlags_None);
    igColorEdit3("fog color", game_state->fog_color.raw, ImGuiColorEditFlags_None);

    igDragFloat("sun attenuation", &game_state->sun_attenuation, 0.1f, 0.0f, MAX_f32, "%.3f", ImGuiSliderFlags_None);
    igDragFloat("halo intensity", &game_state->halo_intensity, 0.1f, 0.0f, MAX_f32, "%.3f", ImGuiSliderFlags_None);

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
    MEM_ZERO_STRUCT(game_state);

    void* memory = game_memory->permenant_storage + sizeof(game_state_s);
    usize remaining_memory = permenant_memory_to_allocate - sizeof(game_state_s);
    
    game_state->shader_arena = partition_permenant_memory(&memory, KILOBYTES(10), &remaining_memory);
    game_state->fbo_arena = partition_permenant_memory(&memory, KILOBYTES(1), &remaining_memory);
    game_state->params_arena = partition_permenant_memory(&memory, KILOBYTES(4), &remaining_memory);
    game_state->texture_arena = partition_permenant_memory(&memory, KILOBYTES(4), &remaining_memory);
    game_state->mesh_arena = partition_permenant_memory(&memory, remaining_memory, &remaining_memory);

    // IO
    create_window(1920, 1080, "water");
    init_input();

    // SHADERS
    init_skybox_shader();
    init_water_shader();
    init_post_processing_shaders();

    game_state->seed = 71892;

    game_state->speed_ramp   = 1.02f;
    game_state->angle_seed   = 0.0f;
    game_state->angle_offset = 0.0f;
    game_state->vertex_drag  = 1.0f;

    for(usize i = 0; i < TOTAL_WAVES; i ++) {
        game_state->waves[i] = (wave_s) {
            .wavelength = 48.0f,
            .amplitude = 4.0f,
            .steepness = 0.3f,
            .speed = 12.0f,

            .direction = VECTOR_2(
                    RAND_IN_RANGE(-1.0f, 1.0f),
                    RAND_IN_RANGE(-1.0f, 1.0f)
                ),

            .w_factor = 0.857f,
            .a_factor = 0.730f,
            .iterations = 4
        };
    }

    // PARAMS FILE
    strcpy(game_state->params_filepath, "ver5");

    // RENDERER
    game_state->camera = (camera_s) {
        .position   = VECTOR_3(0.0f, 12.0f, 0.0f),
        .rotation   = VECTOR_3(0.0f, 180.0f, 0.0f),
        
        .fov        = 70.0f,
        .near_plane = 0.001f,
        .far_plane  = 1500.0f,

        .speed      = 24.0f,
        .sens       = 7500.0f
    };

    game_state->sun = (directional_light_s) {
        .color = VECTOR_RGB(239.0f, 227.0f, 200.0f),
        .direction = VECTOR_3(-0.1f, -0.3f, 0.04f),
        .intensity = 0.9f
    };

    game_state->water_color = VECTOR_RGB(86.0f, 193.0f, 244.0f);

    game_state->tip_color = VECTOR_RGB(34.0f, 115.0f, 120.0f);
    game_state->tip_attenuation = 25.0f;

    game_state->specular_factor = 32.0f;
    game_state->specular_strength = 8.0f;

    game_state->r0 = 0.1f;

    game_state->ambient = 0.18f;
    game_state->ambient_color = VECTOR_RGB(35.0f, 174.0f, 198.0f);

    init_skybox_renderer();
    init_water_renderer();
    init_pproc_renderer();

    // GUI
    init_imgui();

    game_state->water_entity.mesh = primitive_plane_mesh(
        VECTOR_3(-128.0f, 0.0f, -128.0f),
        (v2i) { .x = 1024, .y = 1024},
        VECTOR_2(512.0f, 512.0f),
        &game_state->mesh_arena
    );

    game_state->water_entity.transform = (transform_s) {
        .position = VECTOR_3_ZERO(),
        .rotation = VECTOR_3_ZERO(),
        .scale    = VECTOR_3(1.0f, 0.4f, 1.0f)
    };

    game_state->skybox = create_cubemap((char*[6]) {
            "res/bluecloud_ft.jpg", "res/bluecloud_bk.jpg",
            "res/bluecloud_up.jpg", "res/bluecloud_dn.jpg",
            "res/bluecloud_rt.jpg", "res/bluecloud_lf.jpg"
        }, &game_state->texture_arena);

    game_state->time_scale = 1.0f;

    glfwSetInputMode(game_state->window.glfw_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    // TODO(nix3l): maybe dont give this its own arena, wasting memory
    // look into frame arenas for stuff like this
    load_parameters_from_file(game_state->params_filepath, &game_state->params_arena);
}

static void terminate_game() {
    destroy_mesh(&game_state->water_entity.mesh);
    destroy_mesh(&game_state->skybox_renderer.mesh);
    destroy_mesh(&game_state->pproc_renderer.screen_quad);

    destroy_fbo(&game_state->water_renderer.framebuffer);
    destroy_fbo(&game_state->pproc_renderer.back_buffer);
    destroy_shader(&game_state->water_shader);
    destroy_shader(&game_state->skybox_shader);
    destroy_shader(&game_state->post_processing_shader);
    
    shutdown_imgui();
    destroy_window();

    mem_free(game_memory->transient_storage);
    mem_free(game_memory->permenant_storage);
    mem_free(game_memory);
}

int main(void) {
    init_game_state(GIGABYTES(1), MEGABYTES(16));

    while(!glfwWindowShouldClose(game_state->window.glfw_window)) {
        update_frame_stats();

        update_camera(&game_state->camera);
        render_water(&game_state->water_entity);
        render_skybox(&game_state->skybox, &game_state->water_renderer.framebuffer);
        render_post_processing(&game_state->water_renderer.framebuffer);

        fbo_copy_texture_to_screen(&game_state->water_renderer.framebuffer, GL_COLOR_ATTACHMENT0);

        update_imgui();
        show_debug_stats_window();
        show_settings_window();
        render_imgui();

        glfwSwapBuffers(game_state->window.glfw_window);

        update_input();
        glfwPollEvents();
    }

    terminate_game();

    return 0;
}
