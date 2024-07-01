#version 330 core

in vec3 fs_position;
in vec3 fs_normals;
in vec3 fs_halfway_dir;
in float fs_displacement;

uniform float normal_bias;

uniform vec3 light_dir;
uniform vec3 light_color;
uniform float light_intensity;

uniform vec3 water_color;
uniform vec3 tip_color;

uniform float tip_attenuation;

uniform float specular_factor;
uniform float specular_strength;
uniform vec3 camera_pos;

uniform float r0;

uniform float scatter_amount;
uniform float scatter_angle;
uniform vec3 scatter_color;

uniform float ambient;
uniform vec3 ambient_color;

uniform float reflection_strength;
uniform float env_normal_bias;
uniform samplerCube environment;

out vec3 out_color;

float clamp01(float x) {
    return clamp(x, 0.0, 1.0);
}

void main(void) {
    vec3 color = ambient_color * ambient;
    vec3 normal = fs_normals;
    normal.xz *= normal_bias;
    normal = normalize(normal);

    // DIFFUSE LIGHTING
    float ndotl = dot(normal, -light_dir);
    float ndotl01 = max(ndotl, 0.0);
    float diffuse_factor = max(ndotl, ambient);

    // SPECULAR LIGHTING
    vec3 camera_dir = normalize(camera_pos - fs_position);
    vec3 reflected_light = normalize(reflect(light_dir, normal));
    float vdotr = dot(camera_dir, reflected_light);
    float specular_lighting = specular_strength * ndotl01 * pow(max(vdotr, 0.0), specular_factor);

    // SCHLICK FRESNEL
    float exponential = pow(max(1.0 - max(dot(normal, fs_halfway_dir), 0.0), 0.0), 5.0);
    float fresnel_factor = exponential + (1.0 - exponential) * r0;

    specular_lighting *= fresnel_factor;

    color += water_color * light_color * light_intensity * (diffuse_factor + specular_lighting);

    // ENVIRONMENT REFLECTIONS
    vec3 env_normal = normal;
    env_normal.xz *= env_normal_bias;
    env_normal = normalize(env_normal);

    vec3 reflected_view = normalize(reflect(camera_dir, env_normal));
    vec3 environment_color = texture(environment, reflected_view).rgb;
    color += environment_color * reflection_strength * fresnel_factor;

    // TIP HIGHLIGHTING
    float tip_factor = exp(max(fs_displacement, 0.0) / tip_attenuation) - 1.0;
    color += tip_color * tip_factor;

    // SCATTERING
    // NOTE(nix3l): https://github.com/muckSponge/Optically-Realistic-Water/blob/master/Water/Assets/Water/Shaders/WaterSurface.shader
    // what are these values? who knows!
    float s = max(vdotr * 2.0 - 1.0, 0.0);
    float sa = pow(s, 1.0 / scatter_angle); // exp(s * scatter_angle);
    float scatter_factor = clamp01(s * scatter_amount * clamp01(ndotl * 0.7 + 0.3)) * sa;
    color += scatter_factor * mix(scatter_color * vec3(1.0, 0.4, 0.0), scatter_color, light_color);

    out_color = color;

    // TODO(nix3l): why and how is this needed
    gl_FragDepth = 1.0 - gl_FragCoord.w;
}
