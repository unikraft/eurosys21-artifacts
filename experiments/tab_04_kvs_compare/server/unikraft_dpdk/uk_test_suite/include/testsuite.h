#ifndef UK_TESTSUITE_H
#define UK_TESTSUITE_H

/**
 * Define the test interface for each test suite.
 */
typedef int (*test_runner_t)(void);
typedef int (*test_runner_args_t)(int, char**);

struct uk_ts_fn {
	test_runner_args_t func;
	char *start_marker;
	char *end_marker;
};

#define TESTSUITE_REGISTER(libname, fnc)			\
	static __attribute__((section("testsuite_fn"), used))	\
	test_runner_t  libname ## _ ## fnc = fnc;

#define TESTSUITE_REGISTER_ARGS(libname, fnc, start, end)		\
	static __attribute__((section("testsuite_fn_args"), used))	\
	struct uk_ts_fn libname ## _ ## fnc = {.func = fnc,		\
					       .start_marker = start,	\
					       .end_marker = end};


#endif /* UK_TESTSUITE_H */
