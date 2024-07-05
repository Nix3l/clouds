#version 330 core

layout (location = 0) in vec3 vs_position;

uniform mat4 projview_mat;
uniform mat4 transformation_mat;

void main(void) {
    gl_Position = projview_mat * transformation_mat * vec4(vs_position, 1.0);
}
