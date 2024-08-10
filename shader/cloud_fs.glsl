#version 330 core

// TODO(nix3l): IDEA!! make the step size change depending on the density of the cloud at the point when marching

in vec2 fs_uvs;

#define MAX_CLOUD_MARCH_STEPS   32
#define MAX_SUN_MARCH_STEPS     16

uniform sampler2D scene_tex;
uniform sampler2D depth_tex;

uniform sampler2D blue_noise_tex;
uniform sampler3D noise_tex;

uniform mat4 projection;
uniform mat4 view;

uniform float near_plane;
uniform float far_plane;

uniform vec3 position;
uniform vec3 size;

uniform vec3 camera_pos;
uniform vec3 camera_dir;

uniform float time;

uniform int noise_resolution;

uniform float cloud_scale;
uniform vec3 cloud_offset;
uniform float density_threshold;
uniform float density_multiplier;

uniform float max_march_dist;
uniform int cloud_march_steps;

uniform float absorption;

out vec4 out_color;

// returns the distance to the box (x)
// and distance through the box (y)
vec2 ray_box_distance(vec3 bounds_min, vec3 bounds_max, vec3 ray_origin, vec3 ray_dir) {
    vec3 t0 = (bounds_min - ray_origin) / ray_dir;
    vec3 t1 = (bounds_max - ray_origin) / ray_dir;
    vec3 tmin = min(t0, t1);
    vec3 tmax = max(t0, t1);

    float distA = max(max(tmin.x, tmin.y), tmin.z);
    float distB = min(tmax.x, min(tmax.y, tmax.z));

    // CASE 1: ray originates outside and intersects box (0 <= distA <= distB)
    // distA is to nearest intersection, distB to far intersection

    // CASE 2: ray originates inside and intersects box (distA < 0 < distB)
    // distA is to the nonexistent intersection behind ray, distB is to the real intersection

    // CASE 3: ray misses box (distA > distB)
    
    float dist_to_box = max(0, distA);
    float dist_through_box = max(0, distB - dist_to_box);

    return vec2(dist_to_box, dist_through_box);
}

float sample_density(vec3 position) {
    vec3 uvw = (position + (cloud_offset + vec3(time, time*0.1, time*0.33)) * 10.0) * cloud_scale * 0.01 / noise_resolution;
    return max(0, texture(noise_tex, uvw).r - density_threshold) * density_multiplier;
}

// returns the density at the pixel and the light transimttance to said pixel
vec2 cloud_march(vec3 pixel_dir, vec3 bounds_min, vec3 bounds_max, float box_dist, float interval) {
    vec3 ray_pos = camera_pos - bounds_min + box_dist * pixel_dir;

    int scatter_points = cloud_march_steps > MAX_CLOUD_MARCH_STEPS ? MAX_CLOUD_MARCH_STEPS : cloud_march_steps;
    float step_size = max_march_dist / scatter_points;

    // offset the initial ray march position by a random blue noise value
    // to avoid banding artifacts
    float bn = texture(blue_noise_tex, fs_uvs).r;
    ray_pos += (bn - 0.5) * 2 * step_size;

    float density = 0.0;
    float transmittance = 1.0;
    for(int i = 0; i < MAX_CLOUD_MARCH_STEPS; i ++) {
        if(i >= cloud_march_steps) break; // kind of cheating a dynamic loop condition here

        ray_pos += pixel_dir * step_size;
        float point_density = sample_density(ray_pos);
        float point_transmittance = exp(-point_density * absorption);

        density += point_density;
        transmittance *= point_transmittance;

        // transmittance calculating per point in order to add this optimisation
        // if the transmittance at the current point is too low, then marching further
        // is likely not going to result in a noticeable effect
        if(point_transmittance <= 0.01) break;
    }

    return vec2(density, transmittance);
}

void main(void) {
    vec3 scene_color = texture(scene_tex, fs_uvs).rgb;

    float depth = texture(depth_tex, fs_uvs).r;
    // change depth range [0.0, 1.0] -> [-1.0, 1.0]
    float ndc = depth * 2.0 - 1.0;

    // change the depth to linear world space value
    depth = (2.0 * near_plane * far_plane) / (far_plane + near_plane - ndc * (far_plane - near_plane));

    // get the pixels direction
    // by moving back from clip space -> world space coordinates
    // NOTE(nix3l): should probably be loaded in from the cpu but im lazy
    mat4 unprojection = inverse(projection * mat4(mat3(view)));
    // range of uvs changed from [0.0, 1.0] -> [-1.0, 1.0]
    // -1.0 on the z-axis because we look out on the -ve z-axis
    vec4 clip_space = vec4(fs_uvs * 2.0 - 1.0, -1.0, 1.0);
    // perform perspective division
    clip_space /= ndc;
    // apply unprojection
    vec3 pixel_dir = normalize((unprojection * clip_space).xyz);

    vec3 bounds_min = position - size / 2;
    vec3 bounds_max = position + size / 2;

    vec2 ray_info = ray_box_distance(bounds_min, bounds_max, camera_pos, pixel_dir);
    if(ray_info.y == 0 || ray_info.x > depth) {
        out_color = vec4(scene_color, 1.0);
        return;
    }

    vec2 cloud_info = cloud_march(pixel_dir, bounds_min, bounds_max, ray_info.x, ray_info.y);
    float density = max(0, cloud_info.x);
    float transmittance = cloud_info.y;

    out_color = vec4((scene_color + density) * transmittance, 1.0);
}
