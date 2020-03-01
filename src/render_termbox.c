#include "muapp.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "./termbox/src/termbox.h"

static int text_width(mu_Font font, const char *text, int len) {
    if (len == -1) { len = strlen(text); }
    // TODO: utf8 len
    return len;
}

static int text_height(mu_Font font) {
    return 1;
}

struct tb_ctx {
    struct tb_cell* cells;
    int width;
    int height;

    int sc_left;
    int sc_right;
    int sc_top;
    int sc_bottom;
};

void tb_ctx_init(struct tb_ctx* ctx) {
    ctx->cells = tb_cell_buffer();
    ctx->width = tb_width();
    ctx->height = tb_height();

    ctx->sc_left = 0;
    ctx->sc_right = ctx->width;
    ctx->sc_top = 0;
    ctx->sc_bottom = ctx->height;
}

static void tb_ctx_set(struct tb_ctx* ctx, int x, int y, uint32_t ch, uint16_t* fg, uint16_t* bg) {
    if (x < ctx->sc_left || x >= ctx->sc_right)
        return;
    if (y < ctx->sc_top || y >= ctx->sc_bottom)
        return;

    struct tb_cell* cell = ctx->cells + (y * ctx->width + x);
    cell->ch = ch;

    if (fg) {
        cell->fg = *fg;
    }

    if (bg) {
        cell->bg = *bg;
    }
}

uint16_t tb_col(mu_Color color) {
#if 0
    int c = 0;
    if (color.r > 100)
        c += 1;
    if (color.g > 100)
        c += 2;
    if (color.b > 100)
        c += 4;

    return c;
#endif

    int r = ((int)color.r * 6) / 256;
    int g = ((int)color.g * 6) / 256;
    int b = ((int)color.b * 6) / 256;

    // TODO: 8 colors support
    return 0x10 + r * 36 + g * 6 + b;
}

static void tb_draw_text(struct tb_ctx *ctx, const char *text, mu_Vec2 pos, mu_Color color) {
    int left = pos.x / 8;
    int top = pos.y / 8;
    uint16_t col = tb_col(color);

    uint32_t ch;
    int l;

    while (*text && (l = tb_utf8_char_to_unicode(&ch, text)) > 0) {
        text += l;
        tb_ctx_set(ctx, left, top, ch, &col, NULL);
        ++left;
    }
}

static void tb_draw_rect(struct tb_ctx *ctx, mu_Rect rect, mu_Color color) {
    int left = rect.x / 8;
    int top = rect.y / 8;
    int right = (rect.x + rect.w) / 8;
    int bottom = (rect.y + rect.h) / 8;
    uint16_t col = tb_col(color);
    uint16_t white = TB_WHITE;
    int x, y;

    for (y = top; y < bottom; ++y) {
        for (x = left; x < right; ++x) {
            tb_ctx_set(ctx, x, y, ' ', &white, &col);
        }
    }
}

static void tb_draw_icon(struct tb_ctx *ctx, int id, mu_Rect rect, mu_Color color) {
    int left = rect.x / 8;
    int top = rect.y / 8;
    int right = (rect.x + rect.w) / 8;
    int bottom = (rect.y + rect.h) / 8;
    uint16_t col = tb_col(color);
    uint16_t white = TB_WHITE;
    int x, y;

    for (y = top; y < bottom; ++y) {
        for (x = left; x < right; ++x) {
            tb_ctx_set(ctx, x, y, ' ', &white, &col);
        }
    }

    uint32_t sym = ' ';
    if (id == MU_ICON_CLOSE) {
        sym = 'X';
    } else if (id == MU_ICON_CHECK) {
        sym = 'V';
    } else if (id == MU_ICON_COLLAPSED) {
        sym = '-';
    } else if (id == MU_ICON_EXPANDED) {
        sym = '|';
    }
    tb_ctx_set(ctx, (left + right) / 2, (top + bottom) / 2, sym, &col, &white);
}

static void tb_set_clip_rect(struct tb_ctx *ctx, mu_Rect rect) {
    int left = rect.x / 8;
    int top = rect.y / 8;
    int right = (rect.x + rect.w) / 8;
    int bottom = (rect.y + rect.h) / 8;

    ctx->sc_left = left;
    ctx->sc_right = right;
    ctx->sc_top = top;
    ctx->sc_bottom = bottom;
}

static int tb_run(muapp_t* app, int argc, const char *argv[], muapp_draw_t draw) {
    struct tb_event ev;

    tb_init();
    tb_select_output_mode(TB_OUTPUT_256);
    tb_select_input_mode(TB_INPUT_ESC | TB_INPUT_MOUSE);

    mu_Context *ctx = app->ctx;
    ctx->text_width = text_width;
    ctx->text_height = text_height;

    mu_Style tb_style;
    memcpy(&tb_style, ctx->style, sizeof(tb_style));
    ctx->style = &tb_style;
    tb_style.colors[MU_COLOR_TEXT] = mu_color(0, 0, 0, 0);
    tb_style.colors[MU_COLOR_BORDER] = mu_color(0, 255, 255, 0);
    tb_style.colors[MU_COLOR_WINDOWBG] = mu_color(255, 255, 255, 0);
    tb_style.colors[MU_COLOR_TITLEBG] = mu_color(0, 255, 255, 0);
    tb_style.colors[MU_COLOR_TITLETEXT] = mu_color(0, 0, 0, 0);
    tb_style.colors[MU_COLOR_PANELBG] = mu_color(0, 0, 0, 0);
    tb_style.colors[MU_COLOR_BUTTON] = mu_color(0, 0, 127, 0);
    tb_style.colors[MU_COLOR_BUTTONHOVER] = mu_color(0, 0, 255, 0);
    tb_style.colors[MU_COLOR_BUTTONFOCUS] = mu_color(255, 0, 0, 0);
    tb_style.colors[MU_COLOR_BASE] = mu_color(150, 150, 150, 0);
    tb_style.colors[MU_COLOR_BASEHOVER] = mu_color(150, 150, 150, 0);
    tb_style.colors[MU_COLOR_BASEFOCUS] = mu_color(150, 150, 150, 0);
    tb_style.colors[MU_COLOR_SCROLLBASE] = mu_color(0, 0, 255, 0);
    tb_style.colors[MU_COLOR_SCROLLTHUMB] = mu_color(0, 255, 255, 0);
    tb_style.padding = 8;
    tb_style.spacing = 8;
    tb_style.indent = 8;
    tb_style.title_height = 8;
    tb_style.scrollbar_size = 8;
    tb_style.thumb_size = 8;

    for (;;) {
        // Handle events:
        int evtype = tb_peek_event(&ev, 100);
        switch (evtype) {
        case TB_EVENT_KEY:
            if (ev.key == TB_KEY_ESC) {
                muapp_stop(app, 0);
            } else if (ev.key == TB_KEY_ENTER) {
                mu_input_keydown(ctx, MU_KEY_RETURN);
                mu_input_keyup(ctx, MU_KEY_RETURN);
            } else if (ev.key == TB_KEY_BACKSPACE) {
                mu_input_keydown(ctx, MU_KEY_BACKSPACE);
                mu_input_keyup(ctx, MU_KEY_BACKSPACE);
            } else if (ev.ch) {
				char kbuf[32];
				int l = tb_utf8_unicode_to_char(kbuf, ev.ch);
				if (l > 0) {
					kbuf[l] = 0;
					mu_input_text(ctx, kbuf);
				}
            }
        break;

        case TB_EVENT_MOUSE:
            if (ev.mod == TB_MOD_MOTION) {
                mu_input_mousemove(ctx, ev.x * 8, ev.y * 8);
            }
            if (ev.key == TB_KEY_MOUSE_LEFT) {
                mu_input_mousedown(ctx, ev.x * 8, ev.y * 8, MU_MOUSE_LEFT);
            } else if (ev.key == TB_KEY_MOUSE_RELEASE) {
                mu_input_mouseup(ctx, ev.x * 8, ev.y * 8, MU_MOUSE_LEFT);
            }
        break;

        default:
        break;
        }

        if (app->exit_code != -1000)
            break;

		draw(app);

        tb_clear();

        struct tb_ctx tbctx;
        tb_ctx_init(&tbctx);
        mu_Command *cmd = NULL;
        while (mu_next_command(ctx, &cmd)) {
            switch (cmd->type) {
                case MU_COMMAND_TEXT: tb_draw_text(&tbctx, cmd->text.str, cmd->text.pos, cmd->text.color); break;
                case MU_COMMAND_RECT: tb_draw_rect(&tbctx, cmd->rect.rect, cmd->rect.color); break;
                case MU_COMMAND_ICON: tb_draw_icon(&tbctx, cmd->icon.id, cmd->icon.rect, cmd->icon.color); break;
                case MU_COMMAND_CLIP: tb_set_clip_rect(&tbctx, cmd->clip.rect); break;
            }
        }
        tb_present();
    }

    tb_shutdown();

    return 0;
}

static int tb_is_supported(int argc, const char* argv[]) {
	if (isatty(1))
		return 1;
	return 0;
}

static struct muapp_render_st termbox_render = {
	MUAPP_TEXTMODE,
	tb_is_supported,
	tb_run
};

struct muapp_render_st* muapp_get_render_termbox() {
	return &termbox_render;
};

