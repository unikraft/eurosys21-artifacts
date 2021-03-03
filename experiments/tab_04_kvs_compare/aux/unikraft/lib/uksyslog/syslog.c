#include <syslog.h>
#include <stdio.h>

#define syslog(priority,format,...) printf(format,__VA_ARGS__)
/**
 * TODO:
 * Currently we add support for the console.
 */

void closelog(void)
{
	return;
}

void openlog(const char *ident __unused, int option, int facility)
{
	return;	
}
