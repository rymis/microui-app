#include "renderer.h"

int microui_sdl_run(int argc, const char *argv[], int (*process_frame)(mu_Context *, void*), void* ctx);
int microui_x11_run(int argc, const char *argv[], int (*process_frame)(mu_Context *, void*), void* ctx);
int microui_termbox_run(int argc, const char *argv[], int (*process_frame)(mu_Context *, void*), void* ctx);

int microui_run(int argc, const char *argv[], int (*process_frame)(mu_Context *, void*), void* ctx) {
    return microui_sdl_run(argc, argv, process_frame, ctx);
    //return microui_termbox_run(argc, argv, process_frame, ctx);
}

