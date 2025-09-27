
// 2D OpenGL renderer library
// Currently only supports GLES2
// *Very* barebones and kinda slow

#ifndef CSGL_H_
#define CSGL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <GLES2/gl2.h>

typedef struct csgl_state {
    GLuint simple_sp, font_sp;
    int width, height;
} csgl_state_t;

typedef struct csgl_color {
    unsigned char r, g, b, a;
} csgl_color_t;

#define CSGL_RGBA(r, g, b, a) ((csgl_color_t) { r, g, b, a })
#define CSGL_RGB(r, g, b) ((csgl_color_t) { r, g, b, 255 })
#define CSGL_RGBA_HEX(c) ((csgl_color_t) { ((c) >> 24) & 0xFF, ((c) >> 16) & 0xFF, ((c) >> 8) & 0xFF, (c) & 0xFF })
#define CSGL_RGB_HEX(c) ((csgl_color_t) { ((c) >> 16) & 0xFF, ((c) >> 8) & 0xFF, (c) & 0xFF, 255 })

#define CSGL_WHITE CSGL_RGBA_HEX(0xFFFFFFFF)
#define CSGL_BLACK CSGL_RGBA_HEX(0x000000FF)
#define CSGL_RED CSGL_RGBA_HEX(0xFF0000FF)
#define CSGL_GREEN CSGL_RGBA_HEX(0x00FF00FF)
#define CSGL_BLUE CSGL_RGBA_HEX(0x0000FFFF)
#define CSGL_COOL_GRAY CSGL_RGBA_HEX(0x181818FF)
#define CSGL_TRANSPARENT CSGL_RGBA_HEX(0x00000000)

csgl_state_t csgl_init(int width, int height);
void csgl_deinit(const csgl_state_t *state);

void csgl_scissor_mode(const csgl_state_t *state, int x, int y, int w, int h);
void csgl_reset_scissor_mode(const csgl_state_t *state);

void csgl_clear(const csgl_state_t *state, csgl_color_t color);
void csgl_rect(const csgl_state_t *state, int x, int y, int w, int h, csgl_color_t color);
void csgl_rect_rounded(const csgl_state_t *state, int x, int y, int w, int h, int r, csgl_color_t color);
void csgl_circle(const csgl_state_t *state, int x, int y, int r, csgl_color_t color);

typedef struct csgl_font {
    GLuint texture;
    int texture_w, texture_h;
    void *baked_chars;
    float baseline;
    float fontsize;
    int error, start_char, end_char;
} csgl_font_t;

struct fn_csgl_font_kvargs {
    int fontsize;
    struct {
        int start, end;
    } charset;
    int index;
};

csgl_font_t _csgl_font(const char *path, struct fn_csgl_font_kvargs kv);
csgl_font_t _csgl_font_mem(const unsigned char *data, unsigned long long size, struct fn_csgl_font_kvargs kv);

#ifndef __cplusplus
#define csgl_font(a, ...) _csgl_font(a, (struct fn_csgl_font_kvargs) { .fontsize = 16, .charset = { .start = ' ', .end = '~' }, __VA_ARGS__ })
#define csgl_font_mem(a, b, ...) _csgl_font_mem(a, b, (struct fn_csgl_font_kvargs) { .fontsize = 16, .charset = { .start = ' ', .end = '~' }, __VA_ARGS__ })
#endif

void csgl_font_free(csgl_font_t font);

/* csgl_font("font.ttf")
// csgl_font("font.ttf", .fontsize = 32.0f)
// csgl_font("font.ttf", .charset = { .start = 32, .end = 10000 }) */

struct fn_csgl_text_kvargs {
    csgl_color_t color;
    int spacing;
    int text_length;
    struct { float x, y; } anchor;
};

int _csgl_text(const csgl_state_t *state, int x, int y, csgl_font_t font, const char *string, struct fn_csgl_text_kvargs kv);

#ifndef __cplusplus
#define csgl_text(a, b, c, d, e, ...) _csgl_text(a, b, c, d, e, (struct fn_csgl_text_kvargs) { .color = CSGL_WHITE, .text_length = -1, __VA_ARGS__ })
#endif

int _csgl_measure_text(csgl_font_t font, const char *string, struct fn_csgl_text_kvargs kv);

#ifndef __cplusplus
#define csgl_measure_text(a, b, ...) _csgl_measure_text(a, b, (struct fn_csgl_text_kvargs) { .text_length = -1, __VA_ARGS__ })
#endif

#ifdef __cplusplus
}
#endif

#endif /* CSGL_H_ */
