
#include "csgl.h"

#include <stdio.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

csgl_font_t _csgl_font(const char *path, struct fn_csgl_font_kvargs kv) {
    FILE *f = fopen(path, "rb");
    if (f == NULL) return (csgl_font_t) { .error = 1 };
    fseek(f, 0, 2);
    int size = ftell(f);
    unsigned char *buffer = malloc(size);
    if (buffer == NULL) {
        fclose(f);
        return (csgl_font_t) { .error = 2 };
    }
    fseek(f, 0, 0);
    fread(buffer, size, 1, f);
    fclose(f);
    csgl_font_t font = _csgl_font_mem(buffer, size, kv);
    free(buffer);
    return font;
}

csgl_font_t _csgl_font_mem(const unsigned char *data, unsigned long long size, struct fn_csgl_font_kvargs kv) {
    (void)size; // stb truetype *shrug*
    float width = kv.fontsize * 16, height = kv.fontsize * 16;
    stbtt_fontinfo i;
    if (!stbtt_InitFont(&i, data, 0)) {
        return (csgl_font_t) { .error = 3 };
    }
    
    float scale = stbtt_ScaleForPixelHeight(&i, kv.fontsize);
    int ascent;
    stbtt_GetFontVMetrics(&i, &ascent, NULL, NULL);

    float baseline = ascent * scale;

    unsigned char *buffer = malloc(width * height);
    if (buffer == NULL) return (csgl_font_t) { .error = 2 };

    stbtt_bakedchar *baked_chars = malloc(sizeof(stbtt_bakedchar) * (kv.charset.end - kv.charset.start + 1));
    if (baked_chars == NULL) {
        free(buffer);
        return (csgl_font_t) { .error = 2 };
    }

    int offset = stbtt_GetFontOffsetForIndex(data, kv.index);
    if (offset < 0) {
        free(buffer); free(baked_chars);
        return (csgl_font_t) { .error = 3 };
    }
    int retres = 0;
    // oh god
    while ((retres = stbtt_BakeFontBitmap(data, offset, kv.fontsize, buffer, width, height, kv.charset.start, kv.charset.end - kv.charset.start + 1, baked_chars)) <= 0) {
        height *= 2; width *= 2;
        buffer = realloc(buffer, width * height);
        if (buffer == NULL) {
            free(baked_chars);
            return (csgl_font_t) { .error = 2 };
        }
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, buffer);

    free(buffer);

    return (csgl_font_t) {
        .texture = texture,
        .baked_chars = (void*) baked_chars,
        .fontsize = kv.fontsize,
        .baseline = baseline,
        .start_char = kv.charset.start,
        .end_char = kv.charset.end,
        .texture_w = width,
        .texture_h = height,
    };
}

void csgl_font_free(csgl_font_t font) {
    glDeleteTextures(1, &font.texture);
    free(font.baked_chars);
}

#define MAP_X(x) ((float) (x) / state->width * 2.0f - 1.0f)
#define MAP_Y(y) (-((float) (y) / state->height * 2.0f - 1.0f))
#define MAP_COL(c) (float) c.r / 255, (float) c.g / 255, (float) c.b / 255, (float) c.a / 255

int _csgl_text(const csgl_state_t *state, int _x, int _y, csgl_font_t font, const char *string, struct fn_csgl_text_kvargs kv) {
    GLfloat verts[8], uvs[8];

    float x = _x, y = _y;

    y += font.baseline;
    x -= _csgl_measure_text(font, string, kv) * kv.anchor.x;
    y -= font.fontsize * kv.anchor.y;

    int start_x = x;
    
    glUseProgram(state->font_sp);

    GLint tex = glGetUniformLocation(state->font_sp, "u_texture");
    GLint color = glGetUniformLocation(state->font_sp, "u_color");
    GLint uv = glGetAttribLocation(state->font_sp, "a_uv");
    GLint pos = glGetAttribLocation(state->font_sp, "a_pos");
    
    glUniform4f(color, MAP_COL(kv.color));
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, font.texture);
    glUniform1i(tex, 0);

    glEnableVertexAttribArray(pos);
    glEnableVertexAttribArray(uv);

    int text_idx = 0;
    int codepoint = 0;
    while (kv.text_length < 0 || text_idx < kv.text_length) {
        codepoint = 0;
        unsigned char c = string[text_idx];
        if (c < 0x80) {
            codepoint = c; text_idx += 1;
        } else if ((c & 0xE0) == 0xC0 && text_idx + 1 < kv.text_length) {
            codepoint = ((c & 0x1F) << 6) | (string[text_idx + 1] & 0x3F);
            text_idx += 2;
        } else if ((c & 0xF0) == 0xE0 && text_idx + 2 < kv.text_length) {
            codepoint = ((c & 0xF) << 12) | ((string[text_idx + 1] & 0x3F) << 6) | (string[text_idx + 2] & 0x3F);
            text_idx += 3;
        } else if ((c & 0xF8) == 0xF0 && text_idx + 3 < kv.text_length) {
            codepoint = ((c & 0x7) << 18) | ((string[text_idx + 1] & 0x3F) << 12) | ((string[text_idx + 2] & 0x3F) << 6) | (string[text_idx + 3] & 0x3F);
            text_idx += 4;
        } else {
            text_idx += 1;
            continue;
        }

        if (!codepoint) break;

        if (font.start_char > codepoint || font.end_char < codepoint) {
            if (font.start_char > '?' || font.end_char < '?') continue;
            codepoint = '?';
        }
        
        stbtt_aligned_quad q;
        stbtt_GetBakedQuad(font.baked_chars, font.texture_w, font.texture_h, codepoint - font.start_char, &x, &y, &q, 1);
        
        verts[0] = MAP_X(q.x0); verts[1] = MAP_Y(q.y0); uvs[0] = q.s0; uvs[1] = q.t0;
        verts[2] = MAP_X(q.x1); verts[3] = MAP_Y(q.y0); uvs[2] = q.s1; uvs[3] = q.t0;
        verts[4] = MAP_X(q.x1); verts[5] = MAP_Y(q.y1); uvs[4] = q.s1; uvs[5] = q.t1;
        verts[6] = MAP_X(q.x0); verts[7] = MAP_Y(q.y1); uvs[6] = q.s0; uvs[7] = q.t1;
        
        glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 0, verts);
        glVertexAttribPointer(uv, 2, GL_FLOAT, GL_FALSE, 0, uvs);

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        x += kv.spacing;
    }
    
    glDisableVertexAttribArray(pos);
    glDisableVertexAttribArray(uv);

    return x - start_x;
}

int _csgl_measure_text(csgl_font_t font, const char *string, struct fn_csgl_text_kvargs kv) {
    int text_idx = 0;
    int codepoint = 0;
    float w = 0, _ = 0;
    
    while (kv.text_length < 0 || text_idx < kv.text_length) {
        codepoint = 0;
        unsigned char c = string[text_idx];
        if (c < 0x80) {
            codepoint = c; text_idx += 1;
        } else if ((c & 0xE0) == 0xC0 && text_idx + 1 < kv.text_length) {
            codepoint = ((c & 0x1F) << 6) | (string[text_idx+1] & 0x3F);
            text_idx += 2;
        } else if ((c & 0xF0) == 0xE0 && text_idx + 2 < kv.text_length) {
            codepoint = ((c & 0xF) << 12) | ((string[text_idx+1] & 0x3F) << 6) | (string[text_idx+2] & 0x3F);
            text_idx += 3;
        } else if ((c & 0xF8) == 0xF0 && text_idx + 3 < kv.text_length) {
            codepoint = ((c & 0x7) << 18) | ((string[text_idx+1] & 0x3F) << 12) | ((string[text_idx+2] & 0x3F) << 6) | (string[text_idx+3] & 0x3F);
            text_idx += 4;
        } else {
            text_idx += 1;
            continue;
        }

        if (!codepoint) break;
        
        if (font.start_char > codepoint || font.end_char < codepoint) {
            if (font.start_char > '?' || font.end_char < '?') continue;
            codepoint = '?';
        }

        stbtt_aligned_quad q;
        stbtt_GetBakedQuad(font.baked_chars, font.texture_w, font.texture_h, codepoint - font.start_char, &w, &_, &q, 1);
    }
    
    return w;
}
