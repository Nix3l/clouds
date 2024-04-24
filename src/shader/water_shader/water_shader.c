#include "water_shader.h"

#include "game.h"
#include "util/log.h"

static void bind_attributes() {
    shader_s* shader = &game_state->water_shader;
    shader_bind_attribute(shader, 0, "vs_position");
    shader_bind_attribute(shader, 1, "vs_uvs");
    shader_bind_attribute(shader, 2, "vs_normals");
};

static void load_uniforms(void* data) {
    entity_s* entity = data;

    water_shader_uniforms_s* uniforms = &game_state->water_shader_uniforms;

    // VERTEX SHADER
    shader_load_float(uniforms->time, game_state->curr_time);
    shader_load_mat4(uniforms->transformation, entity_transformation_matrix(entity));
    shader_load_mat4(uniforms->projection_view, camera_projection_view(&game_state->camera));

    for(usize i = 0; i < TOTAL_WAVES; i ++) {
        wave_s wave = game_state->waves[i];

        shader_load_float(uniforms->wavelength + i, wave.wavelength);
        shader_load_float(uniforms->amplitude  + i, wave.amplitude);
        shader_load_float(uniforms->steepness  + i, wave.steepness);
        shader_load_float(uniforms->speed      + i, wave.speed);
        shader_load_vec2 (uniforms->direction  + i, wave.direction);
        shader_load_float(uniforms->w_factor   + i, wave.w_factor);
        shader_load_float(uniforms->a_factor   + i, wave.a_factor);
        shader_load_int  (uniforms->iterations + i, wave.iterations);
    }

    shader_load_float(uniforms->speed_ramp, game_state->speed_ramp);
    shader_load_float(uniforms->angle_seed, RADIANS(game_state->angle_seed));
    shader_load_float(uniforms->angle_offset, RADIANS(game_state->angle_offset));
    shader_load_float(uniforms->vertex_drag, game_state->vertex_drag);

    shader_load_uint(uniforms->seed, game_state->seed);

    // FRAGMENT SHADER
    shader_load_float(uniforms->normal_bias, game_state->normal_bias);

    shader_load_vec3(uniforms->light_dir, game_state->sun.direction);
    shader_load_vec3(uniforms->light_color, game_state->sun.color);
    shader_load_float(uniforms->light_intensity, game_state->sun.intensity);
    
    shader_load_vec3(uniforms->water_color, game_state->water_color);

    shader_load_vec3(uniforms->tip_color, game_state->tip_color);
    shader_load_float(uniforms->tip_attenuation, game_state->tip_attenuation);

    shader_load_float(uniforms->specular_factor, game_state->specular_factor);
    shader_load_float(uniforms->specular_strength, game_state->specular_strength);
    shader_load_vec3(uniforms->camera_pos, game_state->camera.position);

    shader_load_float(uniforms->r0, game_state->r0);

    shader_load_float(uniforms->scatter_amount, game_state->scatter_amount);
    shader_load_float(uniforms->scatter_angle, game_state->scatter_angle);
    shader_load_vec3(uniforms->scatter_color, game_state->scatter_color);

    shader_load_float(uniforms->ambient, game_state->ambient);
    shader_load_vec3(uniforms->ambient_color, game_state->ambient_color);

    shader_load_float(uniforms->env_normal_bias, game_state->env_normal_bias);
    shader_load_float(uniforms->reflection_strength, game_state->reflection_strength);
    shader_load_int(uniforms->environment, 0); // texture slot 0
}

void init_water_shader() {
    game_state->water_shader = load_and_create_shader(
            "forward",
            "shader/forward_vs.glsl",
            "shader/forward_fs.glsl",
            bind_attributes,
            load_uniforms
        );

    water_shader_uniforms_s* uniforms = &game_state->water_shader_uniforms;
    shader_s* shader = &game_state->water_shader;

    // VERTEX SHADER
    uniforms->time              = shader_get_uniform(shader, "time");
    uniforms->transformation    = shader_get_uniform(shader, "transformation");
    uniforms->projection_view   = shader_get_uniform(shader, "projection_view");

    uniforms->wavelength        = shader_get_uniform(shader, "wavelengths[0]");
    uniforms->amplitude         = shader_get_uniform(shader, "amplitudes[0]");
    uniforms->steepness         = shader_get_uniform(shader, "steepnesses[0]");
    uniforms->speed             = shader_get_uniform(shader, "speeds[0]");
    uniforms->direction         = shader_get_uniform(shader, "directions[0]");
    uniforms->w_factor          = shader_get_uniform(shader, "w_factors[0]");
    uniforms->a_factor          = shader_get_uniform(shader, "a_factors[0]");
    uniforms->iterations        = shader_get_uniform(shader, "num_iterations[0]");

    uniforms->speed_ramp        = shader_get_uniform(shader, "speed_ramp");
    uniforms->angle_seed        = shader_get_uniform(shader, "angle_seed");
    uniforms->angle_offset      = shader_get_uniform(shader, "angle_offset");
    uniforms->vertex_drag       = shader_get_uniform(shader, "vertex_drag");

    uniforms->seed              = shader_get_uniform(shader, "seed");

    // FRAGMENT SHADER
    uniforms->normal_bias       = shader_get_uniform(shader, "normal_bias");

    uniforms->light_dir         = shader_get_uniform(shader, "light_dir");
    uniforms->light_color       = shader_get_uniform(shader, "light_color");
    uniforms->light_intensity   = shader_get_uniform(shader, "light_intensity");

    uniforms->water_color       = shader_get_uniform(shader, "water_color");

    uniforms->tip_color         = shader_get_uniform(shader, "tip_color");
    uniforms->tip_attenuation   = shader_get_uniform(shader, "tip_attenuation");

    uniforms->specular_factor   = shader_get_uniform(shader, "specular_factor");
    uniforms->specular_strength = shader_get_uniform(shader, "specular_strength");
    uniforms->camera_pos        = shader_get_uniform(shader, "camera_pos");
    
    uniforms->r0                = shader_get_uniform(shader, "r0");

    uniforms->scatter_amount    = shader_get_uniform(shader, "scatter_amount");
    uniforms->scatter_color     = shader_get_uniform(shader, "scatter_color");
    uniforms->scatter_angle     = shader_get_uniform(shader, "scatter_angle");

    uniforms->ambient           = shader_get_uniform(shader, "ambient");
    uniforms->ambient_color     = shader_get_uniform(shader, "ambient_color");

    uniforms->env_normal_bias     = shader_get_uniform(shader, "env_normal_bias");
    uniforms->reflection_strength = shader_get_uniform(shader, "reflection_strength");
    uniforms->environment         = shader_get_uniform(shader, "environment");
}
