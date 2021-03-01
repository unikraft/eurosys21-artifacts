#include <string.h>
#include <testsuite.h>
#include <test_assert.h>

#define MAX_NR_SPLIT  5

struct strtok_r_tc {
	char *str;
	const char *delim;
	int cnt;
	char *index[MAX_NR_SPLIT];
};

static struct strtok_r_tc strtok_r_data[] = {
	{.str="1:2:3:4", .delim=":", .cnt = 4, .index = {"1", "2", "3", "4"}},
	{.str="1:2:::4", .delim=":", .cnt = 3, .index = {"1", "2", "4"}},
	{.str="1:2:::4", .delim=";", .cnt = 1, .index = {"1:2:::4"}},
};

static int test_strtok_r(void)
{
	char *saveptr;
	char *curptr;
	int cnt = sizeof(strtok_r_data) / sizeof(strtok_r_data[0]);
	int j, i;
	int rc;

	for (i = 0; i < cnt; i++) {
		j = 0;
		curptr = strtok_r(strtok_r_data[i].str, strtok_r_data[i].delim,
				  &saveptr);
		while (j < strtok_r_data[i].cnt) {
			rc = strcmp(strtok_r_data[i].index[j], curptr);
			TEST_ZERO_CHK(rc);
			curptr = strtok_r(NULL, strtok_r_data[i].delim, &saveptr);
			j++;
		}
		TEST_NULL(curptr);
	}
}
TESTSUITE_REGISTER(test_string,test_strtok_r);
