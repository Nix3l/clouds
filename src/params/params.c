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

static void load_param(char* line) {
    char name[32];
    char val[32];
    sscanf(line, "%s [%[^]]", name, val);

    // LOAD PARAM
}

void load_parameters_from_file(char* filepath, arena_s* arena) {
    usize arena_size_before = arena->size;
    usize num_lines = 0;
    char** lines = platform_load_lines_from_file(filepath, &num_lines, arena);

    if(num_lines == 0) return;

    for(usize i = 0; i < num_lines; i ++) {
        char* line = lines[i];
        if(strlen(line) == 0) continue;

        load_param(line);
    }

    // pop any memory that was pushed to the arena in this function
    // not the best way of doing this i think, could be made into something better
    // but what works for now works
    arena_pop(arena, arena->size - arena_size_before);
}

void write_parameters_to_file(char* filepath, arena_s* arena) {
    usize arena_size_before = arena->size;
    char* data = arena_push_to_capacity(arena);
    strcpy(data, "");

    // WRITE PARAMS

    // write the collated data to the given file
    platform_write_to_file(filepath, data, strlen(data), false);

    // release all the unused memory
    arena_pop(arena, arena->capacity - arena_size_before);
}
