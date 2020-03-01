#include <iostream>
#include "muapp.h"

void draw(muapp_t* app) {
    static char buf[1024] = "TEXT";
	mu_Context* ctx = app->ctx;

    mu_begin(ctx);
    int lay[2] = { 60, -1, };
    if (mu_begin_window(ctx, "My Window", mu_rect(5, 5, 400, 200))) {
        mu_layout_row(ctx, 2, lay, 0);

        mu_label(ctx, "First:");
        if (mu_button(ctx, "Button1")) {
            printf("Button1 pressed\n");
        }

        mu_label(ctx, "Second:");
        if (mu_button(ctx, "Button2")) {
            mu_open_popup(ctx, "My Popup");
        }

        if (mu_begin_popup(ctx, "My Popup")) {
            mu_label(ctx, "Hello world!");
            mu_end_popup(ctx);
        }

        mu_label(ctx, "Input:");
        if (mu_textbox(ctx, buf, sizeof(buf)) & MU_RES_SUBMIT) {
            mu_set_focus(ctx, ctx->last_id);
        }

        mu_end_window(ctx);
    } else {
		muapp_stop(app, 0);
	}

    mu_end(ctx);
}

int main(int argc, const char* argv[]) {
	muapp_t app;
	muapp_init(&app);

    return muapp_start(&app, argc, argv, draw);
}

