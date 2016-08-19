#if defined(_WIN32) && !defined(__GNUC__)
#include <stdarg.h>

/* Emulate snprintf() on Windows */
int snprintf(char *buf, size_t len, const char *fmt, ...);

#endif
