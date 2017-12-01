
/* ----------------------------------------------------------------
 *
 * BabyC Toy compiler for educational purposes
 *
 * ---------------------------------------------------------------- */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "log_error.h"

void log_error(char *fmt, ...) {
    va_list ap;

    printf("babyc: ");
    va_start(ap, fmt);
    vprintf(fmt, ap);
    printf("\n");
    va_end(ap);
    exit(1);
}
