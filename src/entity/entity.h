#ifndef ENTITY_H
#define ENTITY_H

#include "base.h"
#include "mesh/mesh.h"

// NOTE(nix3l): no reason to do ecs now for this project
// will implement in future engines if they end up using this code

typedef struct {
    v3f position;
    v3f rotation;
    v3f scale;
} transform_s;

typedef struct {
    transform_s transform;
    mesh_s mesh;
} entity_s;

mat4s entity_transformation_matrix(entity_s* entity);

#endif
