#include "environment.h"
#include "context.h"

// TODO: this is duplicated with assembly.c.
static const int WORD_SIZE = 4;

void new_scope(Context *ctx) {
    // Each function needs a fresh set of local variables (we
    // don't support globals yet).
    environment_free(ctx->env);
    ctx->env = environment_new();

    ctx->stack_offset = -1 * WORD_SIZE;
}

Context *new_context() {
    Context *ctx = malloc(sizeof(Context));
    ctx->stack_offset = 0;
    ctx->env = NULL;
    ctx->label_count = 0;

    return ctx;
}

void context_free(Context *ctx) {
    environment_free(ctx->env);
    free(ctx);
}
