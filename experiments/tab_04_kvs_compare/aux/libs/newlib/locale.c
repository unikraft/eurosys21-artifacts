#include <string.h>
#include <stddef.h>

int strcoll_l(const char *s1 __unused, const char *s2 __unused,
		locale_t locale __unused)
{
	return 0;
}

size_t strxfrm_l(char *restrict s1 __unused, const char *restrict s2 __unused,
			size_t n __unused, locale_t locale __unused)
{
	return 0;
}

int wcscoll_l(const wchar_t *ws1 __unused, const wchar_t *ws2 __unused,
			locale_t locale __unused)
{
	return 0;
}

size_t wcsxfrm_l(wchar_t *restrict ws1 __unused,
			const wchar_t *restrict ws2 __unused,
			size_t n __unused, locale_t locale __unused)
{
	return 0;
}
