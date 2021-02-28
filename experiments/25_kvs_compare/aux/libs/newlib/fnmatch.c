#include <fnmatch.h>

int fnmatch(const char *pat __unused, const char *str __unused, int flags __unused)
{
	return FNM_NOMATCH;
}
