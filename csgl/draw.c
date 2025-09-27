
#include "csgl.h"

#include <math.h>

#define MAP_X(x) ((float) (x) / state->width * 2.0f - 1.0f)
#define MAP_Y(y) (-((float) (y) / state->height * 2.0f - 1.0f))
#define MAP_COL(c) (float) c.r / 255, (float) c.g / 255, (float) c.b / 255, (float) c.a / 255

void csgl_scissor_mode(const csgl_state_t *state, int x, int y, int w, int h) {
    glEnable(GL_SCISSOR_TEST);
    glScissor(x, state->height - y - h, w, h);
}

void csgl_reset_scissor_mode(const csgl_state_t *state) {
    (void)state;
    glDisable(GL_SCISSOR_TEST);
}

void csgl_clear(const csgl_state_t *state, csgl_color_t color) {
    (void)state;
    glClearColor(MAP_COL(color));
    glClear(GL_COLOR_BUFFER_BIT);
}

#define min(a, b) ((a) < (b) ? (a) : (b))

void csgl_rect(const csgl_state_t *state, int x, int y, int w, int h, csgl_color_t color) {
    if (!w || !h) return;

    if (w < 0) { x += w; w = -w; }
    if (h < 0) { y += h; h = -h; }
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > state->width) w = state->width - x;
    if (y + h > state->height) h = state->width - x;
    
    float x1 = MAP_X(x), y1 = MAP_Y(y), x2 = MAP_X(x + w), y2 = MAP_Y(y + h);
    GLfloat vertices[] = {
        x1, y1,
        x2, y1,
        x2, y2,
        x1, y1,
        x1, y2,
        x2, y2,
    };

    glUseProgram(state->simple_sp);
    
    GLint pos_loc = glGetAttribLocation(state->simple_sp, "a_pos");
    glEnableVertexAttribArray(pos_loc);
    glVertexAttribPointer(pos_loc, 2, GL_FLOAT, GL_FALSE, 0, vertices);

    GLint color_loc = glGetUniformLocation(state->simple_sp, "u_color");
    glUniform4f(color_loc, MAP_COL(color));

    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(pos_loc);
}

#define SEGMENTS 6

void csgl_rect_rounded(const csgl_state_t *state, int x, int y, int w, int h, int r, csgl_color_t color) {
    if (!w || !h) return;
    
    if (w < 0) { x += w; w = -w; }
    if (h < 0) { y += h; h = -h; }
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > state->width) w = state->width - x;
    if (y + h > state->height) h = state->height - y;
    float maxr = min(w / 2, h / 2);
    if (r > maxr) r = maxr;
    
    float x1 = MAP_X(x), y1 = MAP_Y(y), x2 = MAP_X(x + w), y2 = MAP_Y(y + h);
    float x1r = MAP_X(x + r), y1r = MAP_Y(y + r), x2r = MAP_X(x + w - r), y2r = MAP_Y(y + h - r);
    GLfloat vertices[24 + (SEGMENTS + 2) * 8] = {
        x1r, y1,
        x2r, y1,
        x2r, y2,
        x1r, y1,
        x1r, y2,
        x2r, y2,
        x1, y1r,
        x2, y1r,
        x2, y2r,
        x1, y1r,
        x1, y2r,
        x2, y2r,
    };
    
    int idx = 24;
    for (int c = 0; c < 4; c++) {
        float initial, ix, iy;
        if (c == 0) { ix = x + w - r; iy = y + r; initial = 1.5f; }
        if (c == 1) { ix = x + w - r; iy = y + h - r; initial = 0.0f; }
        if (c == 2) { ix = x + r; iy = y + r; initial = 1.0f; }
        if (c == 3) { ix = x + r; iy = y + h - r; initial = 0.5f; }
        initial *= M_PI;
        vertices[idx++] = MAP_X(ix);
        vertices[idx++] = MAP_Y(iy);
        for (int i = 0; i < SEGMENTS + 1; i++) {
            float theta = initial + (float) i / SEGMENTS * M_PI * 0.5f;
            float dx = cosf(theta) * r, dy = sinf(theta) * r;
            vertices[idx++] = MAP_X(ix + dx);
            vertices[idx++] = MAP_Y(iy + dy);
        }
    }

    glUseProgram(state->simple_sp);
    
    GLint pos_loc = glGetAttribLocation(state->simple_sp, "a_pos");
    glEnableVertexAttribArray(pos_loc);
    glVertexAttribPointer(pos_loc, 2, GL_FLOAT, GL_FALSE, 0, vertices);

    GLint color_loc = glGetUniformLocation(state->simple_sp, "u_color");
    glUniform4f(color_loc, MAP_COL(color));

    glDrawArrays(GL_TRIANGLES, 0, 12);
    glDrawArrays(GL_TRIANGLE_FAN, 12 + (SEGMENTS + 2) * 0, SEGMENTS + 2);
    glDrawArrays(GL_TRIANGLE_FAN, 12 + (SEGMENTS + 2) * 1, SEGMENTS + 2);
    glDrawArrays(GL_TRIANGLE_FAN, 12 + (SEGMENTS + 2) * 2, SEGMENTS + 2);
    glDrawArrays(GL_TRIANGLE_FAN, 12 + (SEGMENTS + 2) * 3, SEGMENTS + 2);
    
    glDisableVertexAttribArray(pos_loc);
}

void csgl_circle(const csgl_state_t *state, int x, int y, int r, csgl_color_t color) {
    GLfloat vertices[4 + SEGMENTS * 8];
    
    int idx = 0;
    vertices[idx++] = MAP_X(x);
    vertices[idx++] = MAP_Y(y);
    for (int i = 0; i <= SEGMENTS * 4; i++) {
        float theta = (float) i / (SEGMENTS * 4) * M_PI * 2.0f;
        float dx = cosf(theta) * r, dy = sinf(theta) * r;
        vertices[idx++] = MAP_X(x + dx);
        vertices[idx++] = MAP_Y(y + dy);
    }

    glUseProgram(state->simple_sp);

    GLint pos_loc = glGetAttribLocation(state->simple_sp, "a_pos");
    glEnableVertexAttribArray(pos_loc);
    glVertexAttribPointer(pos_loc, 2, GL_FLOAT, GL_FALSE, 0, vertices);

    GLint color_loc = glGetUniformLocation(state->simple_sp, "u_color");
    glUniform4f(color_loc, MAP_COL(color));

    glDrawArrays(GL_TRIANGLE_FAN, 0, 2 + SEGMENTS * 4);
    
    glDisableVertexAttribArray(pos_loc);
}
