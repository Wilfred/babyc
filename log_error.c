
/* ----------------------------------------------------------------
 *
 * BabyC toy compiler
 *
 * ---------------------------------------------------------------- */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "log_error.h"

void log_error(char *fmt, ...) {
    va_list ap;

    printf("babyc: ");
    printf(fmt, ap);
    printf("\n");
    exit(1);
}
