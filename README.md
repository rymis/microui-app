Skeleton for MicroUI based applications.
----------------------------------------

I like the idea of immediate mode GUI, but I found it very difficult to start writing small programs using
IMUI libraries. You have to choose a renderer and to add some build scripts and so on.

So I decided to create small library with simple interface allowing to use MicroUI immediately like this:
``` C
int draw(mu_context_t *ctx);
int main(int argc, const char* argv[]) {
    muapp_t app;
    muapp_init(&app);
    // set some params (optional)
    return muapp_run(argc, argv, draw);
}
```

