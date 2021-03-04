#include <rte_log.h>
#include <stdarg.h>
#include <uk/print.h>
#include <uk/essentials.h>
#include <uk/plat/bootstrap.h>
#include <uk/plat/lcpu.h>

void __rte_panic(const char *funcname, const char *format, ...)
{
	va_list ap;

	uk_pr_crit("PANIC in %s():\n", funcname);
	va_start(ap, format);

	_uk_vprintk(KLVL_CRIT, STRINGIFY(__LIBNAME__), STRINGIFY(__BASENAME__),
		    __LINE__, format, ap);
	va_end(ap);

	ukplat_terminate(UKPLAT_CRASH);
}


void rte_exit(int exit_code, const char *format, ...)
{
	ukplat_halt();
}
