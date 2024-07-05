#include "./math.h"

mat4s get_transformation_matrix(v3f position, v3f rotation, v3f scale) {
    mat4s transformation = MAT4_IDENTITY;
    transformation = glms_translate(transformation, position);
    transformation = glms_rotate_x(transformation, RADIANS(rotation.x));
    transformation = glms_rotate_y(transformation, RADIANS(rotation.y));
    transformation = glms_rotate_z(transformation, RADIANS(rotation.z));
    transformation = glms_scale(transformation, scale);
    return transformation;
}

v3f yaw_pitch_to_direction(f32 yaw, f32 pitch) {
    return VECTOR_3(
            -cosf(RADIANS(pitch)) * sinf(RADIANS(yaw)),
             sinf(RADIANS(pitch)),
             cosf(RADIANS(yaw)) * cosf(RADIANS(pitch))
        );
}

v3f yaw_to_right(f32 yaw) {
    return VECTOR_3(
            cosf(RADIANS(yaw)),
            0,
            sinf(RADIANS(yaw))
        );
}

v3f yaw_pitch_to_up(f32 yaw, f32 pitch) {
    return glms_cross(yaw_pitch_to_direction(yaw, pitch), yaw_to_right(yaw));
}
