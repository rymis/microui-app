#ifndef MU_APPLICATION_H
#define MU_APPLICATION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "microui.h"

#define MUAPP_TEXTMODE 1
#define MUAPP_SOFTRENDER 2

#define MUAPP_MAX_IMAGE_COUNT 16

#define MUAPP_COMMAND_IMG (MU_COMMAND_MAX + 16)

typedef struct muapp_st muapp_t;

typedef struct muapp_image_st {
	int width;
	int height;
	const void* data;
} muapp_image_t;

typedef void (*muapp_draw_t)(muapp_t* app);

struct muapp_render_st {
	unsigned flags;
	int (*is_supported)(int argc, const char* argv[]);
	int (*run)(muapp_t *app,
			int argc, const char* argv[], muapp_draw_t draw);
};

struct muapp_st {
	unsigned flags;
	mu_Context* ctx;
	void* userdata;
	struct muapp_render_st *render;
	int exit_code;
	muapp_image_t images[MUAPP_MAX_IMAGE_COUNT];
	int images_size;
};

int muapp_init(muapp_t* app);
void muapp_free(muapp_t* app);
int muapp_start(muapp_t* app, int argc, const char* argv[], muapp_draw_t draw);
void muapp_stop(muapp_t* app, int code);

// Draw image on the current form.
// Data is the array of pixels as mu_Color
void muapp_draw_image(muapp_t* app, mu_Rect, const void* data);
void muapp_image(muapp_t* app, int width, int height, const void* data);

#ifdef __cplusplus
}
#endif

#endif

