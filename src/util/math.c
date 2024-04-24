#include "./math.h"

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
