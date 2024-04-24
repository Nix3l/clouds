#ifndef PARAMS_H
#define PARAMS_H

#include "memory/memory.h"

void load_parameters_from_file(char* filepath, arena_s* arena);
void write_parameters_to_file(char* filepath, arena_s* arena);

#endif
