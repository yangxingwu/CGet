#include <stdarg.h>

#include "cget.h"

void
DEBUG(LEVEL level, const char *fmt, ...)
{
	char buf[MAXLINE] = {0};
	va_list ap;

	va_start(ap, fmt);

	vsnprintf(buf, sizeof(buf), fmt, ap);

	if (!is_daemon) {
		fputs(buf, stderr);
		fflush(stderr);
	}	

	va_end(ap);

	return;
}
