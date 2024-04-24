#include "entity.h"

mat4s entity_transformation_matrix(entity_s* entity) {
    mat4s transformation = MAT4_IDENTITY;
    transform_s* transform = &entity->transform;
    transformation = glms_translate(transformation, transform->position);
    transformation = glms_rotate_x(transformation, RADIANS(transform->rotation.x));
    transformation = glms_rotate_y(transformation, RADIANS(transform->rotation.y));
    transformation = glms_rotate_z(transformation, RADIANS(transform->rotation.z));
    transformation = glms_scale(transformation, transform->scale);
    return transformation;
}
