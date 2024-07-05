#include "entity.h"

#include "util/math.h"

mat4s entity_transformation_matrix(entity_s* entity) {
    return get_transformation_matrix(entity->transform.position, entity->transform.rotation, entity->transform.scale);
}
