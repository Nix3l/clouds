#version 330 core

// TODO(nix3l): IDEA!! make the step size change depending on the density of the cloud at the point when marching
// TODO(nix3l): IDEA!! add an exponent parameter to the density to decrease banding effects

// TODO(nix3l): figure out better fix for step size
// TODO(nix3l): phase function
// TODO(nix3l): ambient light approximation

in vec2 fs_uvs;

#define MAX_SUN_MARCH_STEPS 12
#define FOUR_PI 12.56637061

uniform sampler2D scene_tex;
uniform sampler2D depth_tex;

uniform sampler2D blue_noise_tex;

uniform sampler2D weather_map_tex;
uniform sampler3D noise_tex;

uniform mat4 projection;
uniform mat4 view;

uniform float near_plane;
uniform float far_plane;

uniform vec3 position;
uniform vec3 size;

uniform vec3 camera_pos;
uniform vec3 camera_dir;

uniform vec3  light_color;
uniform vec3  light_dir;
uniform float light_intensity;

uniform float time;

uniform int noise_resolution;

uniform float cloud_scale;
uniform vec3 cloud_offset;
uniform float density_threshold;
uniform float density_multiplier;

uniform int march_steps;

uniform float march_step_size;
uniform int light_march_steps;

uniform float absorption;
uniform float phase_coefficient;

uniform float edge_falloff;
uniform float height_falloff;

out vec4 out_color;

float remap(float v, float l0, float h0, float ln, float hn) {
    return ln + ((v - l0) * (hn - ln)) / (h0 - l0);
}

float sat(float x) {
    return clamp(x, 0.0, 1.0);
}

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

float weather_map_coverage(vec2 pos) {
    vec2 wm = texture(weather_map_tex, pos / 512).rg;
    return max(wm.r, 2.0 * sat(density_threshold - 0.5) * wm.g);
}

float shape_height_factor(vec3 ray_pos, vec3 bounds_min, vec3 bounds_max) {
    float wh = texture(weather_map_tex, ray_pos.xz / 512).b;
    float ph = (ray_pos.y - bounds_min.y) / (bounds_max.y - bounds_min.y);
    float SRb = sat(remap(ph, 0.0, 0.07, 0.0, 1.0));
    float SRt = sat(remap(ph, wh * 0.2, wh, 1.0, 0.0));

    return SRb * SRt;
}

float density_height_factor(vec3 ray_pos, vec3 bounds_min, vec3 bounds_max) {
    float wd = texture(weather_map_tex, ray_pos.xz / 512).a;
    float ph = (ray_pos.y - bounds_min.y) / (bounds_max.y - bounds_min.y);
    float DRb = ph * sat(remap(ph, 0.0, 0.15, 0.0, 1.0));
    float DRt = sat(remap(ph, 0.9, 1.0, 1.0, 0.0));

    return density_multiplier * DRb * DRt * wd * 2.0;
}

float sample_density(vec3 ray_pos) {
    vec3 uvw = (ray_pos + (cloud_offset + vec3(time, time*0.1, time*0.33)) * 10.0) * cloud_scale * 0.01;
    float coverage = weather_map_coverage(uvw.xz);
    float sh_factor = shape_height_factor(uvw, position - size / 2.0, position + size / 2.0);
    float dh_factor = density_height_factor(uvw, position - size / 2.0, position + size / 2.0);
    float density = texture(noise_tex, uvw / noise_resolution).r;

    return max(0.0, density - density_threshold) * density_multiplier;
    return sat(remap(density * sh_factor, 1.0 - density_threshold * coverage, 1.0, 0.0, 1.0)) * dh_factor;
}

float light_march(vec3 pos) {
    float step_size = 0.5 * size.y / light_march_steps;
    // float step_size = 0.8 * march_step_size;

    // pretty much the same as the way we calculate the transmittance for a pixel
    // except it is towards the sun
    // used for approximating in-scattering
    float transmittance = 1.0;
    vec3 point = pos;
    for(int i = 0; i < MAX_SUN_MARCH_STEPS; i ++) {
        if(i >= light_march_steps) break;

        float density = sample_density(pos);
        transmittance *= exp(-density * step_size * absorption);

        pos += -light_dir * step_size;
    }

    return transmittance;
}

float edge_scale(vec3 ray_pos, vec3 bounds_min, vec3 bounds_max) {
    float x_dist  = min(edge_falloff, min(ray_pos.x - bounds_min.x, bounds_max.x - ray_pos.x));
    float z_dist  = min(edge_falloff, min(ray_pos.z - bounds_min.z, bounds_max.z - ray_pos.z));
    float xz_dist = min(x_dist, z_dist);
    return max(0.0, xz_dist / edge_falloff);
}

float height_gradient(vec3 ray_pos, vec3 bounds_min, vec3 bounds_max) {
    // float y_dist = min(height_falloff, min(ray_pos.x - bounds_min.y, bounds_max.y - ray_pos.y));
    // return max(0.0, y_dist / height_falloff);
    float py = (ray_pos.y - bounds_min.y) / (bounds_max.y - bounds_min.y);
    return exp(-py * height_falloff) * (1.0 - py);
}

// returns the density at the pixel and the light transimttance to said pixel
vec2 cloud_march(vec3 pixel_dir, vec3 bounds_min, vec3 bounds_max, float box_dist, float interval) {
    vec3 ray_pos = camera_pos - bounds_min + box_dist * pixel_dir;

    float dist_travelled = 0.0;

    float step_size = interval / march_steps;
    float max_dist = interval; // < max_march_dist ? interval : max_march_dist;

    // offset the initial ray march position by a random blue noise value
    // to decrease banding artifacts
    float bn = texture(blue_noise_tex, fs_uvs * 16).r;
    // ray_pos += (bn - 0.5) * 2 * step_size;

    float lighting = 0.0;
    float transmittance = 1.0;
    for(int i = 0; dist_travelled < max_dist; i ++) {
        ray_pos += pixel_dir * step_size;
        dist_travelled += step_size;

        float point_density = sample_density(ray_pos * cloud_scale);
        if(point_density > 0.0) {
            // point_density *= edge_scale(ray_pos + bounds_min, bounds_min, bounds_max);
            // point_density *= height_gradient(ray_pos + bounds_min, bounds_min, bounds_max);

            float point_transmittance = exp(-point_density * step_size * absorption);
            float light_transmittance = light_march(ray_pos);

            lighting += point_density * light_transmittance * transmittance * step_size;
            transmittance *= point_transmittance;

            // if the transmittance at the current point is too low, then marching further
            // is likely not going to result in a noticeable effect, so stop
            if(transmittance <= 0.01) break;
        }
    }

    // henyey-greenstein phase coefficient for approximating in-scattering and creating a silver lining effect
    float rdotl = dot(pixel_dir, -light_dir);
    float g = phase_coefficient;
    float p_hg = (1.0 - g * g) / (FOUR_PI * pow(1.0 + g * g - 2.0 * g * rdotl, 1.5));

    return vec2(lighting /*p_hg*/, transmittance);
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
    float lighting = max(0, cloud_info.x);
    float transmittance = cloud_info.y;

    out_color = vec4(scene_color * transmittance + lighting * light_intensity * light_color, 1.0);
}
