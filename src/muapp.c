#include "muapp.h"

#include <string.h>
#include <stdlib.h>

extern struct muapp_render_st* muapp_get_render_sdl(void);
extern struct muapp_render_st* muapp_get_render_termbox(void);
extern struct muapp_render_st* muapp_get_render_x11(void);
extern struct muapp_render_st* muapp_get_render_gdi(void);

typedef struct muapp_render_st* (*get_render_t)(void);
static get_render_t renderers[] = {
	&muapp_get_render_sdl,
	&muapp_get_render_termbox,
	&muapp_get_render_x11,
	&muapp_get_render_gdi,
	NULL
};

int muapp_init(muapp_t* app) {
	if (!app)
		return -1;

	memset(app, 0, sizeof(muapp_t));
	app->ctx = malloc(sizeof(mu_Context));
    mu_init(app->ctx);
	app->exit_code = -1000; // this is the magic value of exit code

	return 0;
}

void muapp_free(muapp_t* app) {
	if (!app)
		return;
	free(app->ctx);
}

int muapp_start(muapp_t* app, int argc, const char* argv[], muapp_draw_t draw) {
	int i;
	// If we don't have renderer yet, we should select it
	if (!app->render) {
		for (i = 0; renderers[i]; ++i) {
			struct muapp_render_st* r = renderers[i]();
			if (!r)
				continue;

			if (app->flags & MUAPP_TEXTMODE) {
				if (!(r->flags & MUAPP_TEXTMODE)) {
					continue;
				}
			}

			if (app->flags & MUAPP_SOFTRENDER) {
				if (!(r->flags & MUAPP_SOFTRENDER)) {
					continue;
				}
			}

			if (!r->is_supported(argc, argv))
				continue;

			app->render = r;
			break;
		}
	}

	if (!app->render) {
		return -1;
	}

	return app->render->run(app, argc, argv, draw);
}

void muapp_stop(muapp_t* app, int code) {
	if (code == -1000)
		code = 1000;
	app->exit_code = code;
}

