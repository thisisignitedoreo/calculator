
#include "csgl.h"

#include <stdlib.h>

const char *simple_vertex =
"attribute vec2 a_pos;"
"void main() {"
    "gl_Position = vec4(a_pos, 0.0, 1.0);"
"}";

const char *simple_fragment =
"precision mediump float;"
"uniform vec4 u_color;"
"void main() {"
    "gl_FragColor = u_color;"
"}";

const char *font_vertex =
"attribute vec2 a_pos;"
"attribute vec2 a_uv;"
"varying vec2 v_uv;"
"void main() {"
    "gl_Position = vec4(a_pos, 0.0, 1.0);"
    "v_uv = a_uv;"
"}";

const char *font_fragment =
"precision mediump float;"
"uniform sampler2D u_texture;"
"uniform vec4 u_color;"
"varying vec2 v_uv;"
"void main() {"
    "gl_FragColor = vec4(u_color.rgb, u_color.a * texture2D(u_texture, v_uv).r);"
"}";
    
#include <stdio.h>
#define CHECK_SHADER(x) do {                          \
    GLint success;                                    \
    glGetShaderiv((x), GL_COMPILE_STATUS, &success);  \
    if (!success) {                                   \
        char log[512];                                \
        glGetShaderInfoLog((x), 512, NULL, log);       \
        printf(#x" compile error:\n%s\n", log);       \
    }                                                 \
} while (0)

csgl_state_t csgl_init(int width, int height) {
    csgl_state_t state = {0};
    state.width = width;
    state.height = height;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    GLuint vert = glCreateShader(GL_VERTEX_SHADER), frag = glCreateShader(GL_FRAGMENT_SHADER);
    GLuint font_vert = glCreateShader(GL_VERTEX_SHADER), font_frag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vert, 1, &simple_vertex, NULL);
    glShaderSource(frag, 1, &simple_fragment, NULL);
    glShaderSource(font_vert, 1, &font_vertex, NULL);
    glShaderSource(font_frag, 1, &font_fragment, NULL);
    glCompileShader(vert);
    glCompileShader(frag);
    glCompileShader(font_vert);
    glCompileShader(font_frag);

    CHECK_SHADER(vert);
    CHECK_SHADER(frag);
    CHECK_SHADER(font_vert);
    CHECK_SHADER(font_frag);
    
    state.simple_sp = glCreateProgram();
    glAttachShader(state.simple_sp, vert);
    glAttachShader(state.simple_sp, frag);
    glLinkProgram(state.simple_sp);
    
    state.font_sp = glCreateProgram();
    glAttachShader(state.font_sp, font_vert);
    glAttachShader(state.font_sp, font_frag);
    glLinkProgram(state.font_sp);

    glDeleteShader(vert);
    glDeleteShader(frag);
    glDeleteShader(font_vert);
    glDeleteShader(font_frag);
    
    return state;
}

void csgl_deinit(const csgl_state_t *state) {
    glDeleteProgram(state->simple_sp);
    glDeleteProgram(state->font_sp);
}
