#include <iostream>
#include "renderer.h"

int draw(mu_Context* ctx, void*) {
    mu_begin(ctx);
    int lay[2] = { 60, -1 };
    if (mu_begin_window(ctx, "My Window", mu_rect(10, 10, 140, 86))) {
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

        mu_end_window(ctx);
        mu_end(ctx);

        return 1;
    }

    mu_end(ctx);

    return 0;
}

int main(int argc, const char* argv[]) {
    return microui_run(argc, argv, draw, NULL);
}

