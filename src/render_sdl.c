#if USE_SDL

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <assert.h>
#include "muapp.h"
#include "atlas.inl"

#define BUFFER_SIZE 16384

struct CTX {
	GLfloat   tex_buf[BUFFER_SIZE *  8];
	GLfloat  vert_buf[BUFFER_SIZE *  8];
	GLubyte color_buf[BUFFER_SIZE * 16];
	GLuint  index_buf[BUFFER_SIZE *  6];

	int width;
	int height;
	int buf_idx;

	SDL_Window *window;
};

static void sdl_destroy(struct CTX* sdl) {
	SDL_DestroyWindow(sdl->window);
}

static int sdl_init(muapp_t* app, struct CTX* sdl) {
	/* init SDL window */
	sdl->width = 800;
	sdl->height = 600;
	sdl->window = SDL_CreateWindow(
			NULL, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			sdl->width, sdl->height, SDL_WINDOW_OPENGL);
	SDL_GL_CreateContext(sdl->window);

	/* init gl */
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	/* init texture */
	GLuint id;
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, ATLAS_WIDTH, ATLAS_HEIGHT, 0,
			GL_ALPHA, GL_UNSIGNED_BYTE, atlas_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	assert(glGetError() == 0);

	return 0;
}

static void sdl_flush(struct CTX* sdl) {
	if (sdl->buf_idx == 0) { return; }

	glViewport(0, 0, sdl->width, sdl->height);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0f, sdl->width, sdl->height, 0.0f, -1.0f, +1.0f);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glTexCoordPointer(2, GL_FLOAT, 0, sdl->tex_buf);
	glVertexPointer(2, GL_FLOAT, 0, sdl->vert_buf);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, sdl->color_buf);
	glDrawElements(GL_TRIANGLES, sdl->buf_idx * 6, GL_UNSIGNED_INT, sdl->index_buf);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	sdl->buf_idx = 0;
}


static void sdl_push_quad(struct CTX* sdl, mu_Rect dst, mu_Rect src, mu_Color color) {
	if (sdl->buf_idx == BUFFER_SIZE) { sdl_flush(sdl); }

	int texvert_idx = sdl->buf_idx *  8;
	int   color_idx = sdl->buf_idx * 16;
	int element_idx = sdl->buf_idx *  4;
	int   index_idx = sdl->buf_idx *  6;
	sdl->buf_idx++;

	/* update texture buffer */
	float x = src.x / (float) ATLAS_WIDTH;
	float y = src.y / (float) ATLAS_HEIGHT;
	float w = src.w / (float) ATLAS_WIDTH;
	float h = src.h / (float) ATLAS_HEIGHT;
	sdl->tex_buf[texvert_idx + 0] = x;
	sdl->tex_buf[texvert_idx + 1] = y;
	sdl->tex_buf[texvert_idx + 2] = x + w;
	sdl->tex_buf[texvert_idx + 3] = y;
	sdl->tex_buf[texvert_idx + 4] = x;
	sdl->tex_buf[texvert_idx + 5] = y + h;
	sdl->tex_buf[texvert_idx + 6] = x + w;
	sdl->tex_buf[texvert_idx + 7] = y + h;

	/* update vertex buffer */
	sdl->vert_buf[texvert_idx + 0] = dst.x;
	sdl->vert_buf[texvert_idx + 1] = dst.y;
	sdl->vert_buf[texvert_idx + 2] = dst.x + dst.w;
	sdl->vert_buf[texvert_idx + 3] = dst.y;
	sdl->vert_buf[texvert_idx + 4] = dst.x;
	sdl->vert_buf[texvert_idx + 5] = dst.y + dst.h;
	sdl->vert_buf[texvert_idx + 6] = dst.x + dst.w;
	sdl->vert_buf[texvert_idx + 7] = dst.y + dst.h;

	/* update color buffer */
	memcpy(sdl->color_buf + color_idx +  0, &color, 4);
	memcpy(sdl->color_buf + color_idx +  4, &color, 4);
	memcpy(sdl->color_buf + color_idx +  8, &color, 4);
	memcpy(sdl->color_buf + color_idx + 12, &color, 4);

	/* update index buffer */
	sdl->index_buf[index_idx + 0] = element_idx + 0;
	sdl->index_buf[index_idx + 1] = element_idx + 1;
	sdl->index_buf[index_idx + 2] = element_idx + 2;
	sdl->index_buf[index_idx + 3] = element_idx + 2;
	sdl->index_buf[index_idx + 4] = element_idx + 3;
	sdl->index_buf[index_idx + 5] = element_idx + 1;
}


static void sdl_draw_rect(struct CTX* sdl, mu_Rect rect, mu_Color color) {
	sdl_push_quad(sdl, rect, atlas[ATLAS_WHITE], color);
}


static void sdl_draw_text(struct CTX* sdl, const char *text, mu_Vec2 pos, mu_Color color) {
	mu_Rect dst = { pos.x, pos.y, 0, 0 };
	for (const char *p = text; *p; p++) {
		if ((*p & 0xc0) == 0x80) { continue; }
		int chr = mu_min((unsigned char) *p, 127);
		mu_Rect src = atlas[ATLAS_FONT + chr];
		dst.w = src.w;
		dst.h = src.h;
		sdl_push_quad(sdl, dst, src, color);
		dst.x += dst.w;
	}
}


static void sdl_draw_icon(struct CTX* sdl, int id, mu_Rect rect, mu_Color color) {
	mu_Rect src = atlas[id];
	int x = rect.x + (rect.w - src.w) / 2;
	int y = rect.y + (rect.h - src.h) / 2;
	sdl_push_quad(sdl, mu_rect(x, y, src.w, src.h), src, color);
}


static int sdl_get_text_width(const char *text, int len) {
	int res = 0;
	for (const char *p = text; *p && len--; p++) {
		if ((*p & 0xc0) == 0x80) { continue; }
		int chr = mu_min((unsigned char) *p, 127);
		res += atlas[ATLAS_FONT + chr].w;
	}
	return res;
}


static int sdl_get_text_height(void) {
  return 18;
}


static void sdl_set_clip_rect(struct CTX* sdl, mu_Rect rect) {
  sdl_flush(sdl);
  glScissor(rect.x, sdl->height - (rect.y + rect.h), rect.w, rect.h);
}

static void sdl_clear(struct CTX* sdl, mu_Color clr) {
  sdl_flush(sdl);
  glClearColor(clr.r / 255., clr.g / 255., clr.b / 255., clr.a / 255.);
  glClear(GL_COLOR_BUFFER_BIT);
}

static void sdl_present(struct CTX* sdl) {
  sdl_flush(sdl);
  SDL_GL_SwapWindow(sdl->window);
}

static const char button_map[256] = {
  [ SDL_BUTTON_LEFT   & 0xff ] =  MU_MOUSE_LEFT,
  [ SDL_BUTTON_RIGHT  & 0xff ] =  MU_MOUSE_RIGHT,
  [ SDL_BUTTON_MIDDLE & 0xff ] =  MU_MOUSE_MIDDLE,
};

static const char key_map[256] = {
  [ SDLK_LSHIFT       & 0xff ] = MU_KEY_SHIFT,
  [ SDLK_RSHIFT       & 0xff ] = MU_KEY_SHIFT,
  [ SDLK_LCTRL        & 0xff ] = MU_KEY_CTRL,
  [ SDLK_RCTRL        & 0xff ] = MU_KEY_CTRL,
  [ SDLK_LALT         & 0xff ] = MU_KEY_ALT,
  [ SDLK_RALT         & 0xff ] = MU_KEY_ALT,
  [ SDLK_RETURN       & 0xff ] = MU_KEY_RETURN,
  [ SDLK_BACKSPACE    & 0xff ] = MU_KEY_BACKSPACE,
};


static int text_width(mu_Font font, const char *text, int len) {
  if (len == -1) { len = strlen(text); }
  return sdl_get_text_width(text, len);
}

static int text_height(mu_Font font) {
  return sdl_get_text_height();
}

static int sdl_run(muapp_t* app, int argc, const char *argv[], muapp_draw_t draw) {
  static float bg[3] = { 90, 95, 100 };
  struct CTX sdl;
  sdl_init(app, &sdl);

  /* init microui */
  mu_Context *ctx = app->ctx;
  ctx->text_width = text_width;
  ctx->text_height = text_height;

  /* main loop */
  for (;;) {
    /* handle SDL events */
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      switch (e.type) {
        case SDL_QUIT: muapp_stop(app, 0); break;
        case SDL_MOUSEMOTION: mu_input_mousemove(ctx, e.motion.x, e.motion.y); break;
        case SDL_MOUSEWHEEL: mu_input_scroll(ctx, 0, e.wheel.y * -30); break;
        case SDL_TEXTINPUT: mu_input_text(ctx, e.text.text); break;

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP: {
          int b = button_map[e.button.button & 0xff];
          if (b && e.type == SDL_MOUSEBUTTONDOWN) { mu_input_mousedown(ctx, e.button.x, e.button.y, b); }
          if (b && e.type ==   SDL_MOUSEBUTTONUP) { mu_input_mouseup(ctx, e.button.x, e.button.y, b);   }
          break;
        }

        case SDL_KEYDOWN:
        case SDL_KEYUP: {
          int c = key_map[e.key.keysym.sym & 0xff];
          if (c && e.type == SDL_KEYDOWN) { mu_input_keydown(ctx, c); }
          if (c && e.type ==   SDL_KEYUP) { mu_input_keyup(ctx, c);   }
          break;
        }
      }
    }

    /* process frame */
	if (app->exit_code != -1000)
		break;

	draw(app);

    /* render */
    sdl_clear(&sdl, mu_color(bg[0], bg[1], bg[2], 255));
    mu_Command *cmd = NULL;
    while (mu_next_command(ctx, &cmd)) {
      switch (cmd->type) {
        case MU_COMMAND_TEXT: sdl_draw_text(&sdl, cmd->text.str, cmd->text.pos, cmd->text.color); break;
        case MU_COMMAND_RECT: sdl_draw_rect(&sdl, cmd->rect.rect, cmd->rect.color); break;
        case MU_COMMAND_ICON: sdl_draw_icon(&sdl, cmd->icon.id, cmd->icon.rect, cmd->icon.color); break;
        case MU_COMMAND_CLIP: sdl_set_clip_rect(&sdl, cmd->clip.rect); break;
      }
    }
    sdl_present(&sdl);
  }

  sdl_destroy(&sdl);

  return 0;
}

static int sdl_is_supported(int argc, const char* argv[]) {
	/* init SDL and renderer */
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		return 0;
	}

	return 1;
}

static struct muapp_render_st sdl_render = {
	MUAPP_TEXTMODE,
	sdl_is_supported,
	sdl_run
};

struct muapp_render_st* muapp_get_render_sdl() {
	return &sdl_render;
}

#else

#include <stdlib.h>

struct muapp_render_st* muapp_get_render_sdl() {
	return NULL;
}

#endif
