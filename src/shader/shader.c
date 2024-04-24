#include "shader.h"

#include "game.h"
#include "util/log.h"
#include "platform/platform.h"

static GLuint compile_shader(char* src_code, GLuint shader_type) {
    GLuint id = glCreateShader(shader_type);

    glShaderSource(id, 1, (const char* const*)&src_code, NULL);
    glCompileShader(id);

    int success;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);

    if(!success) {
        // TODO(nix3l): kinda dumb to have this have a fixed size like this.
        // is there a better solution?
        char log[512];
        glGetShaderInfoLog(id, sizeof(log), NULL, log);
        LOG_ERR("failed to compile %s shader:\n%s\n", shader_type == GL_VERTEX_SHADER ? "vertex" : "fragment", log);
    }

    return id;
}

shader_s create_shader(
        char* filename,
        char* vertex_src, char* fragment_src,
        void (*bind_attributes) (),
        void (*load_uniforms) (void*)) {
    shader_s shader;

    shader.name = filename;
    shader.vertex_full_path = NULL;
    shader.fragment_full_path = NULL;

    shader.program_id = glCreateProgram();
    shader.load_uniforms = load_uniforms;

    shader.vertex_id = compile_shader(vertex_src, GL_VERTEX_SHADER);
    glAttachShader(shader.program_id, shader.vertex_id);

    shader.fragment_id = compile_shader(fragment_src, GL_FRAGMENT_SHADER);
    glAttachShader(shader.program_id, shader.fragment_id);

    // must be called before linking the program unfortunately
    // would have made my life a bit easier if it didnt have to but oh well
    if(bind_attributes) bind_attributes();

    glLinkProgram(shader.program_id);
    glValidateProgram(shader.program_id);

    int success;
    glGetProgramiv(shader.program_id, GL_LINK_STATUS, &success);

    if(!success) {
        char log[512];
        glGetProgramInfoLog(shader.program_id, 512, NULL, log);
        LOG_ERR("failed to link shader:\n%s\n", log);
    }

    // so this is a bit confusing due to the way it works in opengl
    // basically, when a vs/fs/whatever shader is compiled it is its own thing
    // separate from the shader program (pipeline really)
    // after attaching the shader to the program the old compiled shaders still remain
    // so we are wasting memory by keeping them around
    // when they are already linked to the program
    // hence, delete them to free up gpu resources
    glDetachShader(shader.program_id, shader.vertex_id);
    glDetachShader(shader.program_id, shader.fragment_id);

    glDeleteShader(shader.vertex_id);
    glDeleteShader(shader.fragment_id);

    return shader;
}

shader_s load_and_create_shader(
        char* name,
        char* vertex_path, char* fragment_path,
        void (*bind_attributes) (),
        void (*load_uniforms) (void*)) {
    usize vertex_length, fragment_length;
    char* vertex_src = platform_load_text_from_file(vertex_path, &vertex_length, &game_state->shader_arena);
    char* fragment_src = platform_load_text_from_file(fragment_path, &fragment_length, &game_state->shader_arena);

    shader_s shader = create_shader(name, vertex_src, fragment_src, bind_attributes, load_uniforms);
    shader.vertex_full_path = vertex_path;
    shader.fragment_full_path = fragment_path;

    arena_pop(&game_state->shader_arena, vertex_length + fragment_length);

    return shader;
}

void destroy_shader(shader_s* shader) {
    glDeleteProgram(shader->program_id);
}

void shader_start(shader_s* shader) {
    glUseProgram(shader->program_id);
}

void shader_stop() {
    glUseProgram(0);
}

void shader_bind_attribute(shader_s* shader, GLuint attribute, char* attribute_name) {
    glBindAttribLocation(shader->program_id, attribute, attribute_name);
}

GLuint shader_get_uniform(shader_s* shader, char* uniform_name) {
    GLuint id = glGetUniformLocation(shader->program_id, uniform_name);
    
    if(id == (GLuint)-1)
        LOG_ERR("couldnt load uniform with name [%s] in shader [%s]\n", uniform_name, shader->name);

    return id;
}

void shader_load_int(GLuint uniform, i32 value) {
    glUniform1i(uniform, value);
}
  
void shader_load_uint(GLuint uniform, u32 value) {
    glUniform1ui(uniform, value);
}

void shader_load_float(GLuint uniform, f32 value) {
    glUniform1f(uniform, value);
}

void shader_load_bool(GLuint uniform, bool value) {
    glUniform1i(uniform, value ? 1 : 0);
}

void shader_load_vec2(GLuint uniform, vec2s value) {
    glUniform2f(uniform, value.x, value.y);
}

void shader_load_vec3(GLuint uniform, vec3s value) {
    glUniform3f(uniform, value.x, value.y, value.z);
}

void shader_load_mat4(GLuint uniform, mat4s value) {
    glUniformMatrix4fv(uniform, 1, GL_FALSE, (float*) value.raw);
}
