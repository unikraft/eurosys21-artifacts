#include <dlfcn.h>
#include <stddef.h>
#include <uk/essentials.h>

#define DLERROR  "Using a stubbed library"

void *dlopen(const char *filename __unused, int flags __unused)
{
	return NULL;
}

int dlclose(void *handle __unused)
{
	return -1;
}

char *dlerror(void)
{
	return DLERROR;
}
