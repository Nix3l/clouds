#version 330 core

in vec2 fs_uvs;

uniform sampler2D scene_tex;
uniform sampler2D depth_tex;

uniform sampler3D noise_tex;

uniform mat4 projection;
uniform mat4 view;

uniform float near_plane;
uniform float far_plane;

uniform vec3 position;
uniform vec3 size;

out vec4 out_color;

void main(void) {
    vec3 scene_color = texture(scene_tex, fs_uvs).rgb;

    float depth = texture(depth_tex, fs_uvs).r;
    // change depth range [0.0, 1.0] -> [-1.0, 1.0]
    float ndc = depth * 2.0 - 1.0;

    // change the depth to linear world space value
    depth = (2.0 * near_plane * far_plane) / (far_plane + near_plane - ndc * (far_plane - near_plane));
    
    // get the pixels direction
    // by moving back from clip space -> world space coordinates
    // NOTE(nix3l): should probably be loaded in from the cpy but im lazy
    mat4 unprojection = inverse(projection * mat4(mat3(view)));
    // range of uvs changed from [0.0, 1.0] -> [-1.0, 1.0]
    // -1.0 on the z-axis because we look out on the -ve z-axis
    vec4 clip_space = vec4(fs_uvs * 2.0 - 1.0, -1.0, 1.0);
    // perform perspective division
    clip_space /= ndc;
    // apply unprojection
    vec3 pixel_dir = normalize((unprojection * clip_space).xyz);

    float noise = texture(noise_tex, vec3(fs_uvs, 0.0)).r;

    out_color = vec4(vec3(noise), 1.0);
}
