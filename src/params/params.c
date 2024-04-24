#include "params.h"

#include "game.h"
#include "platform/platform.h"
#include "util/log.h"

#define WAVE_IDENTIFIER "wave"

// NOTE(nix3l): not the best implementation honestly im not very happy with it
// even with using the preprocessor its too much boilerplate
// if you are going to reuse this code, please make it better
// for now, it works

#define FMT_U32  "%u"
#define FMT_FLT  "%f"
#define FMT_FLT2 "%f, %f"
#define FMT_FLT3 "%f, %f, %f"

#define GET_PARAM_U32(_parent, _param, _name, _val) do { \
    if(strncmp((_name), #_param, strlen(_name)) == 0) \
        sscanf((_val), FMT_U32, &(_parent)->_param); \
} while(0)

#define GET_PARAM_FLT(_parent, _param, _name, _val) do { \
    if(strncmp((_name), #_param, strlen(_name)) == 0) \
        sscanf((_val), FMT_FLT, &(_parent)->_param); \
} while(0)

#define GET_PARAM_FLT2(_parent, _param, _name, _val) do { \
    if(strncmp((_name), #_param, strlen(_name)) == 0) \
        sscanf((_val), FMT_FLT2, &(_parent)->_param.x, &(_parent)->_param.y); \
} while(0)

#define GET_PARAM_FLT3(_parent, _param, _name, _val) do { \
    if(strncmp((_name), #_param, strlen(_name)) == 0) \
        sscanf((_val), FMT_FLT3, &(_parent)->_param.x, &(_parent)->_param.y, &(_parent)->_param.z); \
} while(0)

#define SET_PARAM_U32(_data, _parent, _param) do { \
    strcat((_data), #_param); \
    strcat((_data), " ["); \
    char _val[256]; \
    MEM_ZERO(_val, sizeof(_val)); \
    snprintf(_val, sizeof(_val), FMT_U32, (_parent)->_param); \
    strcat((_data), _val); \
    strcat((_data), "]\n"); \
} while(0)

#define SET_PARAM_FLT(_data, _parent, _param) do { \
    strcat((_data), #_param); \
    strcat((_data), " ["); \
    char _val[256]; \
    MEM_ZERO(_val, sizeof(_val)); \
    snprintf(_val, sizeof(_val), FMT_FLT, (_parent)->_param); \
    strcat((_data), _val); \
    strcat((_data), "]\n"); \
} while(0)

#define SET_PARAM_FLT2(_data, _parent, _param) do { \
    strcat((_data), #_param); \
    strcat((_data), " ["); \
    char _val[256]; \
    MEM_ZERO(_val, sizeof(_val)); \
    snprintf(_val, sizeof(_val), FMT_FLT2, (_parent)->_param.x, (_parent)->_param.y); \
    strcat((_data), _val); \
    strcat((_data), "]\n"); \
} while(0)

#define SET_PARAM_FLT3(_data, _parent, _param) do { \
    strcat((_data), #_param); \
    strcat((_data), " ["); \
    char _val[256]; \
    MEM_ZERO(_val, sizeof(_val)); \
    snprintf(_val, sizeof(_val), FMT_FLT3, (_parent)->_param.x, (_parent)->_param.y, (_parent)->_param.z); \
    strcat((_data), _val); \
    strcat((_data), "]\n"); \
} while(0)

static usize load_wave(char** lines, usize i, usize wave_index) {
    wave_s* wave = &game_state->waves[wave_index];

    char* line;
    while((line = lines[++i])) {
        if(*line == '}') return i;

        char name[32];
        char value[32];
        sscanf(line, "\t%s [%[^]]", name, value);

        // thank you preprocessor
        GET_PARAM_FLT (wave, wavelength, name, value);
        GET_PARAM_FLT (wave, amplitude,  name, value);
        GET_PARAM_FLT (wave, steepness,  name, value);
        GET_PARAM_FLT (wave, speed,      name, value);
        GET_PARAM_FLT2(wave, direction,  name, value);
        GET_PARAM_FLT (wave, w_factor,   name, value);
        GET_PARAM_FLT (wave, a_factor,   name, value);
        GET_PARAM_U32 (wave, iterations, name, value);
    }

    // there should not be a situation in which this is reached
    LOG_WARN("reached unreachable part of function, there might be a problem in the parameters file.");
    return i;
}

static void load_param(char* line) {
    char name[32];
    char val[32];
    sscanf(line, "%s [%[^]]", name, val);

    GET_PARAM_FLT3(game_state, water_entity.transform.scale, name, val);

    GET_PARAM_U32 (game_state, seed,                name, val);
    GET_PARAM_FLT (game_state, angle_seed,          name, val);
    GET_PARAM_FLT (game_state, angle_offset,        name, val);
    GET_PARAM_FLT (game_state, vertex_drag,         name, val);
    GET_PARAM_FLT (game_state, speed_ramp,          name, val);
    GET_PARAM_FLT (game_state, normal_bias,         name, val);
    GET_PARAM_FLT3(game_state, water_color,         name, val);
    GET_PARAM_FLT3(game_state, tip_color,           name, val);
    GET_PARAM_FLT (game_state, tip_attenuation,     name, val);
    GET_PARAM_FLT (game_state, specular_factor,     name, val);
    GET_PARAM_FLT (game_state, specular_strength,   name, val);
    GET_PARAM_FLT (game_state, r0,                  name, val);
    GET_PARAM_FLT (game_state, ambient,             name, val);
    GET_PARAM_FLT3(game_state, ambient_color,       name, val);
    GET_PARAM_FLT3(game_state, sun.color,           name, val);
    GET_PARAM_FLT3(game_state, sun.direction,       name, val);
    GET_PARAM_FLT (game_state, sun.intensity,       name, val);
    GET_PARAM_FLT (game_state, time_scale,          name, val);
    GET_PARAM_FLT (game_state, scatter_amount,      name, val);
    GET_PARAM_FLT3(game_state, scatter_color,       name, val);
    GET_PARAM_FLT (game_state, scatter_angle,       name, val);
    GET_PARAM_FLT (game_state, env_normal_bias,     name, val);
    GET_PARAM_FLT (game_state, reflection_strength, name, val);
    GET_PARAM_FLT (game_state, fog_distance,        name, val);
    GET_PARAM_FLT (game_state, fog_ramp,            name, val);
    GET_PARAM_FLT (game_state, fog_attenuation,     name, val);
    GET_PARAM_FLT3(game_state, fog_color,           name, val);
    GET_PARAM_FLT (game_state, sun_attenuation,     name, val);
    GET_PARAM_FLT (game_state, halo_intensity,      name, val);
}

void load_parameters_from_file(char* filepath, arena_s* arena) {
    usize arena_size_before = arena->size;
    usize num_lines = 0;
    char** lines = platform_load_lines_from_file(filepath, &num_lines, arena);

    if(num_lines == 0) return;

    for(usize i = 0; i < num_lines; i ++) {
        char* line = lines[i];
        if(strlen(line) == 0) continue;

        // wave blocks
        if(strncmp(line, WAVE_IDENTIFIER, strlen(WAVE_IDENTIFIER)) == 0) {
            u32 wave_index;
            sscanf(line, "wave %u {", &wave_index);

            // TODO(nix3l): skipping an extra wave block doesnt work yet

            // if there are more waves than expected,
            // skip the current wave block and process
            // whatever comes afterwards
            if(wave_index >= TOTAL_WAVES) {
                while(i < num_lines)
                    if(*lines[i++] == '}')
                        break;

                continue;
            }

            // assign i to the return value of this function
            // to pick up where it left off
            i = load_wave(lines, i, wave_index);
            continue;
        }

        load_param(line);
    }

    // pop any memory that was pushed to the arena in this function
    // not the best way of doing this i think, could be made into something better
    // but what works for now works
    arena_pop(arena, arena->size - arena_size_before);
}

static void write_wave_params(char* data, wave_s* wave) {
    strcat(data, "\t");
    SET_PARAM_FLT(data, wave, wavelength);
    strcat(data, "\t");
    SET_PARAM_FLT(data, wave, amplitude);
    strcat(data, "\t");
    SET_PARAM_FLT(data, wave, steepness);
    strcat(data, "\t");
    SET_PARAM_FLT(data, wave, speed);
    strcat(data, "\t");
    SET_PARAM_FLT2(data, wave, direction);
    strcat(data, "\t");
    SET_PARAM_FLT(data, wave, w_factor);
    strcat(data, "\t");
    SET_PARAM_FLT(data, wave, a_factor);
    strcat(data, "\t");
    SET_PARAM_U32(data, wave, iterations);
}

void write_parameters_to_file(char* filepath, arena_s* arena) {
    usize arena_size_before = arena->size;
    char* data = arena_push_to_capacity(arena);
    strcpy(data, "");

    // write waves
    for(usize i = 0; i < TOTAL_WAVES; i ++) {
        wave_s* wave = &game_state->waves[i];

        strcat(data, WAVE_IDENTIFIER);

        char wave_index[12];
        snprintf(wave_index, sizeof(wave_index), " %lu {\n", i);
        strcat(data, wave_index);

        write_wave_params(data, wave);
        strcat(data, "}\n");
    }

    // write params
    SET_PARAM_FLT3(data, game_state, water_entity.transform.scale);
    SET_PARAM_U32 (data, game_state, seed);
    SET_PARAM_FLT (data, game_state, angle_seed);
    SET_PARAM_FLT (data, game_state, angle_offset);
    SET_PARAM_FLT (data, game_state, vertex_drag);
    SET_PARAM_FLT (data, game_state, speed_ramp);
    SET_PARAM_FLT (data, game_state, normal_bias);
    SET_PARAM_FLT3(data, game_state, water_color);
    SET_PARAM_FLT3(data, game_state, tip_color);
    SET_PARAM_FLT (data, game_state, tip_attenuation);
    SET_PARAM_FLT (data, game_state, specular_factor);
    SET_PARAM_FLT (data, game_state, specular_strength);
    SET_PARAM_FLT (data, game_state, r0);
    SET_PARAM_FLT (data, game_state, ambient);
    SET_PARAM_FLT3(data, game_state, ambient_color);
    SET_PARAM_FLT3(data, game_state, sun.color);
    SET_PARAM_FLT3(data, game_state, sun.direction);
    SET_PARAM_FLT (data, game_state, sun.intensity);
    SET_PARAM_FLT (data, game_state, time_scale);
    SET_PARAM_FLT (data, game_state, scatter_amount);
    SET_PARAM_FLT (data, game_state, scatter_angle);
    SET_PARAM_FLT3(data, game_state, scatter_color);
    SET_PARAM_FLT (data, game_state, env_normal_bias);
    SET_PARAM_FLT (data, game_state, reflection_strength);
    SET_PARAM_FLT (data, game_state, fog_distance);
    SET_PARAM_FLT (data, game_state, fog_ramp);
    SET_PARAM_FLT (data, game_state, fog_attenuation);
    SET_PARAM_FLT3(data, game_state, fog_color);
    SET_PARAM_FLT (data, game_state, sun_attenuation);
    SET_PARAM_FLT (data, game_state, halo_intensity);

    // write the collated data to the given file
    platform_write_to_file(filepath, data, strlen(data), false);

    // release all the unused memory
    // NOTE(nix3l): i could just clear this
    // but that would be assuming we would always be giving this function
    // a dedicated params arena
    // which is not guaranteed
    arena_pop(arena, arena->capacity - arena_size_before);
}
