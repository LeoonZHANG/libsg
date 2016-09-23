/**
 * snprintf.h
 * Cross paltform snprintf.
 */

#include <sg/str/snprintf.h>

#if defined(_WIN32) && !defined(__GNUC__)

#include <stdarg.h>
#include <sg/platform/windows/snprintf.h>


/* Microsoft has finally implemented snprintf in Visual Studio 2015, use c99-snprintf instead of for older version. */
#if defined(_MSC_VER) && _MSC_VER < 1900
#define snprintf rpl_snprintf
#endif

/* Emulate snprintf() on Windows, _snprintf() doesn't zero-terminate the buffer on overflow... */
int sg_snprintf(char *buf, size_t len, const char *fmt, ...)
{
	va_list ap;
	int n;

	va_start(ap, fmt);
	n = snprintf(buf, len, fmt, ap);
	va_end(ap);

	/* It's a sad fact of life that no one ever checks the return value of
	* snprintf(). Zero-terminating the buffer hopefully reduces the risk
	* of gaping security holes.
	*/
	if (n < 0)
		if (len > 0)
			buf[0] = '\0';

	return n;
}

#endif
