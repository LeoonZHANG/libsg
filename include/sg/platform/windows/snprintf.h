#include "../../sys/os.h"
#include "../../util/compiler.h"

#if defined(SG_OS_WINDOWS) && !defined(SG_COMPILER_GCC)
#include <stdarg.h>

/* Emulate snprintf() on Windows */
int snprintf(char *buf, size_t len, const char *fmt, ...);

#endif
