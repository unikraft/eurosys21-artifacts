#include <stdio.h>
#include <testsuite.h>
#include <uk/essentials.h>
#include <string.h>

extern test_runner_t __start_testsuite_fns, __end_testsuite_fns;
extern struct uk_ts_fn *__start_testsuite_arg_fns, *__end_testsuite_arg_fns;

int main(__unused int argc, __unused char * argv[])
{
	test_runner_t *fns;

	printf("Called main with %d args %s\n", argc, argv[0]);
	fns = &__start_testsuite_fns;
	while (fns < &__end_testsuite_fns) {
		printf("Executing instruction at %p\n", fns);
		(*fns)();
		fns++;
	}

	struct uk_ts_fn *arg_fns;
	arg_fns = &__start_testsuite_arg_fns;
	while (arg_fns < &__end_testsuite_arg_fns) {
		int i = 0;
		int start_index = 0, end_index = 0, cnt = 0;
		for (i = 1; i < argc; i++) {
			if (!start_index && !strcmp(argv[i], arg_fns->start_marker)) {
				start_index = i;
				cnt++;
			} else if (!end_index && !strcmp(argv[i],
						arg_fns->end_marker)) {
				end_index = i;
				break;
			} else if (start_index)
				cnt++;
		}
		printf("Executing instruction at %p\n", arg_fns->func);
		if (start_index == 0) {
			cnt = 1;
		}
		(*arg_fns->func)(cnt, &argv[start_index]);
		arg_fns++;
	}
}
