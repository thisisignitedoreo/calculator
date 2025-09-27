#include <EGL/egl.h>
#include <csgl.h>
#include <cril.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

typedef struct {
    char *data;
    size_t size, capacity;
} sb_t;

#define da_push(a, e) do {                                  \
    if (!(a)->data) {                                       \
        (a)->data = malloc(8);                              \
        (a)->capacity = 8;                                  \
    }                                                       \
    if ((a)->size == (a)->capacity) {                       \
        (a)->data = realloc((a)->data, (a)->capacity * 2);  \
        (a)->capacity *= 2;                                 \
    }                                                       \
    (a)->data[(a)->size++] = (e);                           \
} while (0)

#define da_push_many(a, pt) do {                            \
    char *p = (char*)(pt);                                  \
    while (*p) da_push(a, *(p++));                          \
} while (0)

#define da_free(a) do {                                     \
    free((a)->data);                                        \
} while (0)

sb_t expression = {0};

const char *double_to_str(double num, int *size_ptr) {
    static char memstr[64];
    double integ, frac = modf(num, &integ);
    int size;
    if (frac < 0.000000000001) size = snprintf(memstr, 64, "%.0F", integ);
    else {
        size = snprintf(memstr, 64, "%.12F", num);
        while (size > 0 && memstr[size-1] == '0') size--;
    }
    if (size < 64) memstr[size] = 0;
    if (size_ptr) *size_ptr = size;
    return memstr;
}

#include "calc.c"

typedef struct {
    csgl_color_t bg, bgp1, bright_bgp1, bgp2, bright_bgp2, dim_fg, fg, red;
} theme_t;

theme_t gruber_darker = {
    .bg =          CSGL_RGB_HEX(0x181818),
    .bgp1 =        CSGL_RGB_HEX(0x282828),
    .bright_bgp1 = CSGL_RGB_HEX(0x383838),
    .bgp2 =        CSGL_RGB_HEX(0x453d41),
    .bright_bgp2 = CSGL_RGB_HEX(0x484848),
    .dim_fg =      CSGL_RGB_HEX(0x72696e),
    .fg =          CSGL_RGB_HEX(0xe4e4ef),
    .red =         CSGL_RGB_HEX(0xf43841),
};

theme_t nord = {
    .bg =          CSGL_RGB_HEX(0x2e3440),
    .bgp1 =        CSGL_RGB_HEX(0x3b4252),
    .bright_bgp1 = CSGL_RGB_HEX(0x434c5e),
    .bgp2 =        CSGL_RGB_HEX(0x434c5e),
    .bright_bgp2 = CSGL_RGB_HEX(0x4c566a),
    .dim_fg =      CSGL_RGB_HEX(0x929aa9),
    .fg =          CSGL_RGB_HEX(0xeceff4),
    .red =         CSGL_RGB_HEX(0xbf616a),
};

theme_t gruvbox = {
    .bg =          CSGL_RGB_HEX(0x282828),
    .bgp1 =        CSGL_RGB_HEX(0x3c3836),
    .bright_bgp1 = CSGL_RGB_HEX(0x504945),
    .bgp2 =        CSGL_RGB_HEX(0x665c54),
    .bright_bgp2 = CSGL_RGB_HEX(0x7c6f64),
    .dim_fg =      CSGL_RGB_HEX(0xa89984),
    .fg =          CSGL_RGB_HEX(0xebdbb2),
    .red =         CSGL_RGB_HEX(0xfb4934),
};

theme_t noir = {
    .bg =          CSGL_RGB_HEX(0x000000),
    .bgp1 =        CSGL_RGB_HEX(0x121212),
    .bright_bgp1 = CSGL_RGB_HEX(0x303030),
    .bgp2 =        CSGL_RGB_HEX(0x303030),
    .bright_bgp2 = CSGL_RGB_HEX(0x585858),
    .dim_fg =      CSGL_RGB_HEX(0x787878),
    .fg =          CSGL_RGB_HEX(0xeeeeee),
    .red =         CSGL_RGB_HEX(0xff0000),
};

theme_t catppuccin = {
    .bg =          CSGL_RGB_HEX(0x1e1e2e),
    .bgp1 =        CSGL_RGB_HEX(0x313244),
    .bright_bgp1 = CSGL_RGB_HEX(0x45475a),
    .bgp2 =        CSGL_RGB_HEX(0x585b70),
    .bright_bgp2 = CSGL_RGB_HEX(0x6c7086),
    .dim_fg =      CSGL_RGB_HEX(0x9399b2),
    .fg =          CSGL_RGB_HEX(0xcdd6f4),
    .red =         CSGL_RGB_HEX(0xf38ba8),
};

theme_t *theme = &gruber_darker;

int main() {
    EGLDisplay dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (dpy == EGL_NO_DISPLAY) { puts("ERROR: eglGetDisplay(): No display"); return 1; }
    if (!eglInitialize(dpy, NULL, NULL)) { eglTerminate(dpy); puts("ERROR: eglInitialize()"); return 1; }

    EGLConfig cfg; EGLint ncfg;
    eglChooseConfig(dpy, (EGLint[]) {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_DEPTH_SIZE, 16,
        EGL_SAMPLE_BUFFERS, 1,
        EGL_SAMPLES, 4,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE,
    }, &cfg, 1, &ncfg);

    EGLSurface surf = eglCreateWindowSurface(dpy, cfg, 0, NULL);
    
    int width, height;
    
    if (eglQuerySurface(dpy, surf, EGL_WIDTH, &width) == EGL_FALSE) {
        eglDestroySurface(dpy, surf);
        eglTerminate(dpy);
        puts("ERROR: eglQuerySurface()");
        return 1;
    }
    
    if (eglQuerySurface(dpy, surf, EGL_HEIGHT, &height) == EGL_FALSE) {
        eglDestroySurface(dpy, surf);
        eglTerminate(dpy);
        puts("ERROR: eglQuerySurface()");
        return 1;
    }
    
    EGLContext ctx = eglCreateContext(dpy, cfg, EGL_NO_CONTEXT, (EGLint[]) { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE });
    eglMakeCurrent(dpy, surf, surf, ctx);

    FILE *f = fopen("savestate", "rb");
    if (f) {
        char t = 0;
        fread(&t, 1, 1, f);
        if (t == 0) theme = &gruber_darker;
        if (t == 1) theme = &nord;
        if (t == 2) theme = &gruvbox;
        if (t == 3) theme = &noir;
        if (t == 4) theme = &catppuccin;
        fclose(f);
    }
    
    csgl_state_t state = csgl_init(width, height);

    csgl_font_t font_32 = csgl_font("ProggyDotted Regular.ttf", .fontsize = 48);
    if (font_32.error) {
        eglDestroyContext(dpy, ctx);
        eglDestroySurface(dpy, surf);
        eglTerminate(dpy);
        printf("ERROR: csgl_font(): Error code 0x%02X\n", font_32.error);
        return 1;
    }
    
    csgl_font_t font_24 = csgl_font("ProggyDotted Regular.ttf", .fontsize = 32);
    if (font_24.error) {
        eglDestroyContext(dpy, ctx);
        eglDestroySurface(dpy, surf);
        eglTerminate(dpy);
        printf("ERROR: csgl_font(): Error code 0x%02X\n", font_24.error);
        return 1;
    }

    const char *id_path = getenv("CALC_INPUT_PSEUDOFILE");
    cril_id_t id = cril_init(id_path ? id_path : "/dev/input/event1");

    const char *buttons[] = {
        "BS", "CLR", "MR", "MC", "M+", "M-",
        "+", "-", "(", "7", "8", "9",
        "/", "*", ")", "4", "5", "6",
        "=", ".", "0", "1", "2", "3",
    };
    
    const char *buttons_2[] = {
        "^", "!", "sqrt(x)", "lg10(x)",
        ",", "pi", "cbrt(x)", "ln(x)",
        "%", "e", "rt(x,y)", "lg(x,y)",
    };
    
    cril_input_t input;
    bool quit = false;
    int selected_x = 0, selected_y = 1;
    float x_scroll = 0; bool adjust_x = false;
    bool evaluated = false; double mem = 0;
    bool menu = false; int menu_selected = 0;
    int x, y, w, h, r; csgl_color_t color;
    bool fps_counter = false;
    struct timespec previous, delta;
    while (true) {
        clock_gettime(CLOCK_MONOTONIC, &previous);
        while (cril_read_input(id, &input)) {
            if (input.type == CRIL_HOLD) {
                if (input.key == BTN_TL2) fps_counter = !fps_counter;
                if (!menu) {
                    if (input.key == BTN_TL) menu = true;
                    if (input.key == BTN_X && expression.size) expression.size--;
                    if (input.key == BTN_TR) { evaluate(&expression); evaluated = true; }
                    if (input.key == BTN_A) {
                        if ((error || evaluated) && !(selected_y == 4 && selected_x == 0) && !(selected_y == 1 && (selected_x == 4 || selected_x == 5))) {
                            expression.size = 0;
                            x_scroll = 0.0f;
                        }
                        error = false; evaluated = false;
                        if (selected_y == 1) {
                            switch (selected_x % 6) {
                                case 0: if (expression.size) expression.size--; break;
                                case 1: expression.size = 0; break;
                                case 2: {
                                    int size;
                                    const char *memstr = double_to_str(mem, &size);
                                    for (int i = 0; i < size; i++) da_push(&expression, memstr[i]);
                                } break;
                                case 3: mem = 0.0; break;
                                case 4: mem += evaluate(&expression); evaluated = true; break;
                                case 5: mem -= evaluate(&expression); evaluated = true; break;
                            }
                        } else if (selected_x < 6) {
                            switch (selected_y) {
                                case 2: da_push(&expression, *buttons[6 + selected_x]); break;
                                case 3: da_push(&expression, *buttons[12 + selected_x]); break;
                                case 4: switch (selected_x) {
                                    case 0: evaluate(&expression); evaluated = true; break;
                                    default: da_push(&expression, *buttons[18 + selected_x]); break;
                                } break;
                            }
                        } else {
                            if (selected_x < 8) da_push_many(&expression, buttons_2[(selected_y - 2) * 4 + selected_x - 6]);
                            else if (selected_x == 9 && selected_y == 2) da_push_many(&expression, "lg(10,");
                            else if (selected_x == 9 && selected_y == 3) da_push_many(&expression, "lg(e,");
                            else {
                                const char *str = buttons_2[(selected_y - 2) * 4 + selected_x - 6];
                                while (*str) {
                                    da_push(&expression, *str);
                                    if (*str == '(') break;
                                    str++;
                                }
                            }
                        }
                        if (evaluated) x_scroll = 0;
                        adjust_x = true;
                    }
                } else {
                    if (input.key == BTN_TL || input.key == BTN_B) menu = false;
                    if (input.key == BTN_A) {
                        if (menu_selected == 0) menu = false;
                        if (menu_selected == 1) theme = &gruber_darker;
                        if (menu_selected == 2) theme = &nord;
                        if (menu_selected == 3) theme = &gruvbox;
                        if (menu_selected == 4) theme = &noir;
                        if (menu_selected == 5) theme = &catppuccin;
                        if (menu_selected == 6) quit = true;
                    }
                }
            }

            if (input.type == CRIL_ANALOG) {
                if (!menu) {
                    if (input.axis == ABS_HAT0X && input.value > 0 && selected_y > 1) selected_x = (selected_x + 1) % 10;
                    else if (input.axis == ABS_HAT0X && input.value > 0 && selected_y > 0) selected_x = (selected_x + 1) % 12;
                    if (input.axis == ABS_HAT0X && input.value < 0 && selected_y > 1) selected_x = selected_x == 0 ? 9 : selected_x - 1;
                    else if (input.axis == ABS_HAT0X && input.value < 0 && selected_y > 0) selected_x = selected_x == 0 ? 11 : selected_x - 1;
                    if (input.axis == ABS_HAT0X && input.value > 0 && selected_y == 0) { x_scroll += 16; adjust_x = true; }
                    if (input.axis == ABS_HAT0X && input.value < 0 && selected_y == 0) x_scroll = x_scroll < 16 ? 0 : x_scroll - 16;
                    if (input.axis == ABS_HAT0Y && input.value > 0) {
                        selected_y = (selected_y + 1) % 5;
                        if (selected_x > 7 && selected_y == 2) selected_x = (selected_x - 8) / 2 + 8;
                    }
                    if (input.axis == ABS_HAT0Y && input.value < 0) {
                        if (selected_x > 7 && selected_y == 2) selected_x = (selected_x - 8) * 2 + 8;
                        selected_y = selected_y == 0 ? 4 : selected_y - 1;
                    }
                } else {
                    if (input.axis == ABS_HAT0Y && input.value > 0) menu_selected = (menu_selected + 1) % 7;
                    if (input.axis == ABS_HAT0Y && input.value < 0) menu_selected = menu_selected > 0 ? menu_selected - 1 : 6;
                }
            }
        }

        csgl_clear(&state, theme->bg);

        x = 8; y = 8; w = width * 4 / 6 - 16; h = height / 5 - 16; r = 16;
        if (selected_y == 0) color = theme->bright_bgp2;
        else color = theme->bgp2;
        
        csgl_rect_rounded(&state, x, y, w, h, r, color);
        csgl_scissor_mode(&state, x, y, w, h);
        if (error) {
            csgl_text(&state, 24 - x_scroll, 8 + (height / 5 - 16) / 2, font_32, "Error", .color = theme->red, .anchor = { .y = 0.5f });
        } else if (expression.size) {
            int x = 24 - x_scroll, y = 8 + (height / 5 - 16) / 2;
            int number_size = 0;
            for (size_t i = 0; i < expression.size;) {
                while (i < expression.size && (isalnum(expression.data[i]) || expression.data[i] == '.')) {
                    number_size++; i++;
                }
                if (number_size) {
                    x += csgl_text(&state, x, y, font_32, expression.data + i - number_size, .text_length = number_size, .color = theme->fg, .anchor = { .y = 0.5f });
                } else {
                    x += csgl_text(&state, x, y, font_32, expression.data + i, .text_length = 1, .color = theme->dim_fg, .anchor = { .y = 0.5f }); i++;
                }
                bool pad = (i >= expression.size || expression.data[i] != ')') && expression.data[i - 1] != '(';
                pad = pad && (i >= expression.size || expression.data[i] != ',');
                pad = pad && (i >= expression.size || expression.data[i] != '!');
                pad = pad && (!isalpha(expression.data[i - 1]) || (i >= expression.size || expression.data[i] != '('));
                if (pad) x += 8;
                number_size = 0;
            }
            if (adjust_x && x > width * 4 / 6 - 24) x_scroll += x - (width * 4 / 6 - 24);
            adjust_x = false;
        } else {
            csgl_text(&state, 24 - x_scroll, 8 + (height / 5 - 16) / 2, font_32, "Start typing...", .color = theme->dim_fg, .anchor = { .y = 0.5f });
        }
        csgl_reset_scissor_mode(&state);

        x = 8 + width * 4 / 6; y = 8;
        w = width * 2 / 6 - 16; h = height / 5 - 16;
        r = 16; color = theme->bgp2;
        csgl_rect_rounded(&state, x, y, w, h, r, color);
        
        csgl_scissor_mode(&state, x, y, w, h);
        int size;
        const char *memstr = double_to_str(mem, &size);
        csgl_text(&state, x + w/2, y + h/2, font_32, memstr, .text_length = size, .color = theme->fg, .anchor = { 0.5f, 0.5f });
        csgl_reset_scissor_mode(&state);

        for (int x = 0; x < 6; x++) {
            if (x == selected_x % 6 && selected_y == 1) color = theme->bright_bgp1;
            else color = theme->bgp1;
            csgl_rect_rounded(&state, width * x / 6 + 8, height / 5 + 8, width / 6 - 16, height / 5 - 16, 16, color);

            const char *text = buttons[x];
            
            csgl_text(&state, width * x / 6 + 8 + (width / 6 - 16) / 2, height / 5 + 8 + (height / 5 - 16) / 2, font_32, text, .color = theme->fg, .anchor = { 0.5f, 0.5f });
        }
        
        if (selected_x < 6) {
            for (int x = 0; x < 6; x++) {
                for (int y = 2; y < 5; y++) {
                    if (x == selected_x && y == selected_y) color = theme->bright_bgp1;
                    else color = theme->bgp1;
                    csgl_rect_rounded(&state, width * x / 6 + 8, height * y / 5 + 8, width / 6 - 16, height / 5 - 16, 16, color);

                    const char *text = buttons[(y - 1) * 6 + x];
                    
                    csgl_text(&state, width * x / 6 + 8 + (width / 6 - 16) / 2, height * y / 5 + 8 + (height / 5 - 16) / 2, font_32, text, .color = theme->fg, .anchor = { 0.5f, 0.5f });
                }
            }
        } else {
            for (int y = 2; y < 5; y++) {
                int px = 0;
                for (int x = 0; x < 4; x++) {
                    if (6 + x == selected_x && y == selected_y) color = theme->bright_bgp1;
                    else color = theme->bgp1;
                    if (x < 2) w = width / 6 - 16;
                    else w = width / 3 - 16;
                    csgl_rect_rounded(&state, px + 8, height * y / 5 + 8, w, height / 5 - 16, 16, color);

                    const char *text = buttons_2[(y - 2) * 4 + x];
                    
                    csgl_text(&state, px + 8 + w / 2, height * y / 5 + height / 10, font_32, text, .color = theme->fg, .anchor = { 0.5f, 0.5f });
                    
                    if (x < 2) px += width / 6;
                    else px += width / 3;
                }
            }
        }

        if (selected_x < 6) {
            csgl_circle(&state, width / 2 - 6, height - 8, 4, theme->fg);
            csgl_circle(&state, width / 2 + 6, height - 8, 4, theme->bright_bgp1);
        } else {
            csgl_circle(&state, width / 2 - 6, height - 8, 4, theme->bright_bgp1);
            csgl_circle(&state, width / 2 + 6, height - 8, 4, theme->fg);
        }

        if (menu) {
            y = 8;
            float menu_width = csgl_measure_text(font_24, "Gruber Darker") + 32 + 16;
            csgl_rect(&state, 0, 0, width, height, CSGL_RGBA_HEX(0x0000007F));
            csgl_rect(&state, 0, 0, menu_width, height, theme->bg);

            csgl_text(&state, 8, y, font_24, "Calculator", .color = theme->fg); y += 32;
            csgl_text(&state, 8, y, font_24, "by aciddev_", .color = theme->dim_fg); y += 32;
            y += 16;

            if (menu_selected == 0) csgl_rect(&state, 0, y, menu_width, 32, theme->bgp1);
            csgl_text(&state, 8, y, font_24, "Continue", .color = theme->fg);
            y += 32 + 8;
            
            csgl_text(&state, 8, y, font_24, "Themes:", .color = theme->dim_fg);
            y += 32;

#define THEME_ENTRY(id, nm, st) \
            if (menu_selected == (id)) csgl_rect(&state, 0, y, menu_width, 32, theme->bgp1); \
            csgl_circle(&state, 16, y + 16, 8, theme == &(st) ? theme->fg : menu_selected == (id) ? theme->dim_fg : theme->bgp2); \
            if (theme != &(st)) csgl_circle(&state, 16, y + 16, 6, menu_selected == (id) ? theme->bgp1 : theme->bg); \
            csgl_text(&state, 32, y, font_24, (nm), .color = theme->fg); \
            y += 32

            THEME_ENTRY(1, "Gruber Darker", gruber_darker);
            THEME_ENTRY(2, "Nord", nord);
            THEME_ENTRY(3, "Gruvbox", gruvbox);
            THEME_ENTRY(4, "Noir", noir);
            THEME_ENTRY(5, "Catppuccin", catppuccin);

            y += 8;
            
            if (menu_selected == 6) csgl_rect(&state, 0, y, menu_width, 32, theme->bgp1);
            csgl_text(&state, 8, y, font_24, "Quit", .color = theme->fg);
        }

        if (fps_counter) {
            static char buf[64];
            csgl_text(&state, 8, 8, font_24, buf, .text_length = snprintf(buf, 64, "FPS: %.2lf", 1.0 / ((double) delta.tv_sec + delta.tv_nsec * 1e-9)));
        }
        
        eglSwapBuffers(dpy, surf);

        if (quit) break;
        
        struct timespec current;
        clock_gettime(CLOCK_MONOTONIC, &current);

        delta = (struct timespec) { current.tv_sec - previous.tv_sec, current.tv_nsec - previous.tv_nsec }; (void) delta;
        if (delta.tv_nsec < 16666666) nanosleep(&(struct timespec) { 0, 16666666 - delta.tv_nsec < 0 ? 0 : 16666666 - delta.tv_nsec }, NULL);
    }

    da_free(&expression);

    csgl_font_free(font_32);
    csgl_font_free(font_24);
    csgl_deinit(&state);

    f = fopen("savestate", "wb");
    if (f) {
        char t = 0;
        if (theme == &gruber_darker) t = 0;
        if (theme == &nord) t = 1;
        if (theme == &gruvbox) t = 2;
        if (theme == &noir) t = 3;
        if (theme == &catppuccin) t = 4;
        fwrite(&t, 1, 1, f);
        fclose(f);
    }
    
    eglDestroyContext(dpy, ctx);
    eglDestroySurface(dpy, surf);
    eglTerminate(dpy);

    return 0;
}
