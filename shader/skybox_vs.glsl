#version 330 core

layout (location = 0) in vec3 vs_position;

uniform mat4 projection_view;

out vec3 fs_texcoord;

void main(void) {
    gl_Position = projection_view * vec4(vs_position, 1.0);
    gl_Position = gl_Position.xyww;

    fs_texcoord = vs_position;
}
