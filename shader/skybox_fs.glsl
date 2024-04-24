#version 330 core

in vec3 fs_texcoord;

uniform samplerCube cubemap;

out vec4 out_color;

void main(void) {
    out_color = texture(cubemap, fs_texcoord);
}
