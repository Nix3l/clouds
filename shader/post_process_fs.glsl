#version 330 core

in vec2 fs_uvs;
in vec3 fs_direction;

uniform sampler2D scene_tex;
uniform sampler2D depth_tex;

uniform float near_plane;
uniform float far_plane;

uniform float fog_distance;
uniform float fog_ramp;
uniform float fog_attenuation;
uniform vec3  fog_color;

uniform float sun_attenuation;
uniform float sun_intensity;
uniform float halo_intensity;
uniform vec3  sun_color;
uniform vec3  sun_direction;

uniform mat4 projection;
uniform mat4 view;

out vec4 out_color;

void main(void) {
    vec3 color = vec3(0.0);

    vec3 scene_color = texture(scene_tex, fs_uvs).rgb;

    float depth = texture(depth_tex, fs_uvs).r;
    // change depth range [0.0, 1.0] -> [-1.0, 1.0]
    float ndc = depth * 2.0 - 1.0;

    // change the depth to linear world space value
    depth = (2.0 * near_plane * far_plane) / (far_plane + near_plane - ndc * (far_plane - near_plane));
    
    // SUN
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
    vec3 sun_dir   = normalize(-sun_direction);
    float sun_factor = pow(max(dot(pixel_dir, sun_dir), 0.0), sun_attenuation);

    // FOG
    float fog_factor = exp2((fog_ramp / sqrt(log(2.0))) * depth - fog_distance);
    float height_factor = pow(max(pixel_dir.y, 0.0), 1.0 / fog_attenuation);
    color = mix(scene_color, fog_color, clamp(fog_factor + height_factor, 0.0, 1.0));
    color = mix(color, scene_color, height_factor);

    color += sun_color * sun_factor * sun_intensity * halo_intensity;

    out_color = vec4(color, 1.0);
}
