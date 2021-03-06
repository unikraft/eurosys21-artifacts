#include <limits.h>
#include <errno.h>
#include <stdlib.h>
#include <test_assert.h>
#include <stdio.h>

struct test_param {
	char *param_string;
	long result_value;
	int iserror;
	int base;
	int end_idx;
	char *ptr_chk;
	int  test_valid;
};

static struct test_param param[] = {
	{.param_string="10", .result_value=10, .iserror=0, .base=10,
	 .end_idx=2, .ptr_chk=NULL, .test_valid=0xF},
	{.param_string="+10", .result_value=10, .iserror=0, .base=10,
	 .end_idx=3, .ptr_chk=NULL, .test_valid=0xF},
	{.param_string="-10", .result_value=-10, .iserror=0, .base=10,
	 .end_idx=3, .ptr_chk=NULL, .test_valid=0x3},
	{.param_string="0x10", .result_value=16, .iserror=0, .base=16,
	 .end_idx=4, .ptr_chk=NULL, .test_valid=0xF},
	{.param_string="   10", .result_value=10, .iserror=0, .base=10,
	 .end_idx=5, .ptr_chk=NULL, .test_valid=0xF},
	{.param_string="1 0", .result_value=1, .iserror=0, .base=10,
	 .end_idx=1, .ptr_chk=NULL, .test_valid=0xF},
	{.param_string="    ", .result_value=0, .iserror=0, .base=10,
	 .end_idx=0, .ptr_chk=NULL, .test_valid=0xF},
	{.param_string="0xFFFFFFFF", .result_value=0xFFFFFFFF, .iserror=0,
	 .base=16, .end_idx=10, .ptr_chk=NULL, .test_valid=0xF},
	{.param_string="0xFFFFFFFFFFFFFFFFF", .result_value=0xFFFFFFFFFFFFFFFF,
	 .iserror=ERANGE, .base=16, .end_idx=19, .ptr_chk=NULL,
	 .test_valid=0xC},
	{.param_string="0xFFFFFFFFFFFFFFFF", .result_value=ULONG_MAX,
	 .iserror=0, .base=16, .end_idx=18, .ptr_chk=NULL,
	 .test_valid=0xC},
	{.param_string="-0xFFFFFFFFFFFFFFFF", .result_value=LONG_MIN,
	 .iserror=ERANGE, .base=16, .end_idx=19, .ptr_chk=NULL,
	 .test_valid=0x3},
};

int test_strtol(void)
{
	int i = 0;
	int len = sizeof(param)/ sizeof(param[0]);
	long int result = 0;


	for (i = 0; i < len; i++) {

		if (param[i].test_valid & 0x1) {
			errno = 0;
			printf("Test Case: %d, string: %s, expected result: %ld, error_case: %d, base: %d, space count: %d, ptr check %p\n",
					i, param[i].param_string,
					param[i].result_value,
					param[i].iserror, param[i].base,
					param[i].end_idx,
					param[i].ptr_chk);

			result = strtol(param[i].param_string,
					&param[i].ptr_chk,
					param[i].base);

			printf("Error code %d\n", errno);
			if (param[i].iserror) {
				printf("Expecting an error: %d, %d\n",
					param[i].iserror,
						errno);
				TEST_EXPR(param[i].iserror == errno);
			} else {
				TEST_ZERO_CHK(errno);
			}

			if (!param[i].iserror) {
				printf("Expecting a result: %8lx, %8lx\n",
						param[i].result_value, result);
				TEST_EXPR((result == param[i].result_value));
			} else if(param[i].iserror == -EINVAL) {
				printf("Expecting result to be zero\n");
				TEST_ZERO_CHK(result);
			} else if (param[i].iserror == -ERANGE) {
				printf("Overflow / Underflow occured\n");
				TEST_EXPR(result == LONG_MIN ||
					  result == LONG_MAX);
			}

			printf("%p --- %p\n", param[i].ptr_chk,
					(&param[i].param_string[0] +
					 param[i].end_idx));
			TEST_EXPR(param[i].ptr_chk ==
					(&param[i].param_string[0] +
					 param[i].end_idx));
		}
	}

	return 0;
}

int test_strtoul(void)
{
	int i = 0;
	int len = sizeof(param)/ sizeof(param[0]);
	unsigned long int result = 0;


	for (i = 0; i < len; i++) {

		if (param[i].test_valid & 0x4) {

			errno = 0;
			printf("Test Case: %d, string: %s, expected result: %ld, error_case: %d, base: %d, space count: %d, ptr check %p\n",
					i, param[i].param_string,
					param[i].result_value,
					param[i].iserror, param[i].base,
					param[i].end_idx,
					param[i].ptr_chk);

			result = strtoul(param[i].param_string,
					 &param[i].ptr_chk,
					 param[i].base);

			printf("Error code %d\n", errno);
			if (param[i].iserror) {
				printf("Expecting an error: %d, %d\n",
					param[i].iserror,
						errno);
				TEST_EXPR(param[i].iserror == errno);
			} else {
				TEST_ZERO_CHK(errno);
			}

			if (!param[i].iserror) {
				printf("Expecting a result: %8lx, %8lx\n",
						param[i].result_value, result);
				TEST_EXPR((result == param[i].result_value));
			} else if(param[i].iserror == -EINVAL) {
				printf("Expecting result to be zero\n");
				TEST_ZERO_CHK(result);
			} else if (param[i].iserror == -ERANGE) {
				printf("Overflow / Underflow occured\n");
				TEST_EXPR(result == LONG_MIN ||
					  result == LONG_MAX);
			}

			printf("%p --- %p\n", param[i].ptr_chk,
					(&param[i].param_string[0] +
					 param[i].end_idx));
			TEST_EXPR(param[i].ptr_chk ==
					(&param[i].param_string[0] +
					 param[i].end_idx));
		}
	}

	return 0;
}
int test_strtoll(void)
{
	int i = 0;
	int len = sizeof(param)/ sizeof(param[0]);
	long long int result = 0;


	for (i = 0; i < len; i++) {

		if (param[i].test_valid & 0x2) {

			printf("Test Case: %d, string: %s, expected result: %ld, error_case: %d, base: %d, space count: %d, ptr check %p\n",
					i, param[i].param_string,
					param[i].result_value,
					param[i].iserror, param[i].base,
					param[i].end_idx,
					param[i].ptr_chk);

			result = strtoll(param[i].param_string,
					&param[i].ptr_chk,
					param[i].base);

			printf("Error code %d\n", errno);
			if (param[i].iserror) {
				printf("Expecting an error: %d, %d\n",
					param[i].iserror,
						errno);
				TEST_EXPR(param[i].iserror == errno);
			} else {
				TEST_ZERO_CHK(errno);
			}

			if (!param[i].iserror) {
				printf("Expecting a result: %8lx, %8lx\n",
						param[i].result_value, result);
				TEST_EXPR((result == param[i].result_value));
			} else if(param[i].iserror == -EINVAL) {
				printf("Expecting result to be zero\n");
				TEST_ZERO_CHK(result);
			} else if (param[i].iserror == -ERANGE) {
				printf("Overflow / Underflow occured\n");
				TEST_EXPR(result == LONG_MIN ||
					  result == LONG_MAX);
			}

			printf("%p --- %p\n", param[i].ptr_chk,
					(&param[i].param_string[0] +
					 param[i].end_idx));
			TEST_EXPR(param[i].ptr_chk ==
					(&param[i].param_string[0] +
					 param[i].end_idx));
		}
	}

	return 0;
}
int test_strtoull(void)
{
	int i = 0;
	int len = sizeof(param)/ sizeof(param[0]);
	unsigned long long int result = 0;


	for (i = 0; i < len; i++) {

		if (param[i].test_valid & 0x8) {

			errno = 0;
			printf("Test Case: %d, string: %s, expected result: %ld, error_case: %d, base: %d, space count: %d, ptr check %p\n",
					i, param[i].param_string,
					param[i].result_value,
					param[i].iserror, param[i].base,
					param[i].end_idx,
					param[i].ptr_chk);

			result = strtoull(param[i].param_string,
					&param[i].ptr_chk,
					param[i].base);

			printf("Error code %d\n", errno);
			if (param[i].iserror) {
				printf("Expecting an error: %d, %d\n",
					param[i].iserror,
						errno);
				TEST_EXPR(param[i].iserror == errno);
			} else {
				TEST_ZERO_CHK(errno);
			}

			if (!param[i].iserror) {
				printf("Expecting a result: %8lx, %8lx\n",
						param[i].result_value, result);
				TEST_EXPR((result == param[i].result_value));
			} else if(param[i].iserror == -EINVAL) {
				printf("Expecting result to be zero\n");
				TEST_ZERO_CHK(result);
			} else if (param[i].iserror == -ERANGE) {
				printf("Overflow / Underflow occured\n");
				TEST_EXPR(result == LONG_MIN ||
					  result == LONG_MAX);
			}

			printf("%p --- %p\n", param[i].ptr_chk,
					(&param[i].param_string[0] +
					 param[i].end_idx));
			TEST_EXPR(param[i].ptr_chk ==
					(&param[i].param_string[0] +
					 param[i].end_idx));
		}
	}

	return 0;
}
