#ifndef LIGHT_H
#define LIGHT_H

#include "base.h"

typedef struct {
    v3f color;
    v3f direction;

    f32 intensity;
} directional_light_s;

#endif
