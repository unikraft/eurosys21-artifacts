#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/syscall.h>
#include "syscall-x86_64.h"
#include "tsc.h"

#ifndef STRINGIFY
#ifndef __STRINGIFY
#define __STRINGIFY(x) #x
#endif
#define STRINGIFY(x) __STRINGIFY(x)
#endif

#ifndef CONFIG_WARMUP
#define CONFIG_WARMUP 1
#endif
#ifndef CONFIG_BENCHTSC
#define CONFIG_BENCHTSC 1
#endif
#ifndef CONFIG_BENCHTSC_AND_LOOP
#define CONFIG_BENCHTSC_AND_LOOP 0
#endif
#ifndef CONFIG_NB_ROUNDS
#define CONFIG_NB_ROUNDS 20
#endif
#ifndef CONFIG_ITER_PER_ROUND
#define CONFIG_ITER_PER_ROUND 1000000
#endif

#ifndef __NR_noop
#define __NR_noop 500
#endif

#define sys_noop()     syscall0(__NR_noop)

__attribute__((noinline)) long function_noop()
{
	/* keep the call from being optimized away */
	asm volatile("");
	return 0xC0FFEE;
}


uint64_t syscall_bench(uint64_t iter)
{
	uint64_t t0, t1;

	t0 = bench_start();
	while (iter-- > 0)
		sys_noop();
	t1 = bench_end();
	return t1 - t0;
}

uint64_t funcall_bench(uint64_t iter)
{
	uint64_t t0, t1;

	t0 = bench_start();
	while (iter-- > 0)
		function_noop();
	t1 = bench_end();
	return t1 - t0;
}

#if CONFIG_BENCHTSC
uint64_t tsc_bench(uint64_t iter)
{
	uint64_t t0, t1;

	t0 = bench_start();
#if CONFIG_BENCHTSC_AND_LOOP
	while (iter-- > 0) { asm volatile(""); }
#endif
	t1 = bench_end();
	return t1 - t0;
}
#else
#define tsc_bench(...) (0)
#endif

int _print_cfg_bool(const char *name, int set)
{
	if (set)
		return printf("%s=y\n", name);
	return printf("# %s is not set\n", name);
}
#define print_cfg_bool(x) \
	_print_cfg_bool(#x, (x))

int _print_cfg_int(const char *name, int val)
{
	return printf("%s=%d\n", name, val);
}
#define print_cfg_int(x) \
	_print_cfg_int(#x, (x))

int main()
{
    static uint64_t overhead_tsc[CONFIG_NB_ROUNDS];
    static uint64_t overhead_scall[CONFIG_NB_ROUNDS];
    static uint64_t overhead_fcall[CONFIG_NB_ROUNDS];
    uint64_t i;

    printf("--- 8< ---\n");
    print_cfg_bool(CONFIG_WARMUP);
    print_cfg_bool(CONFIG_BENCHTSC);
    print_cfg_bool(CONFIG_BENCHTSC_AND_LOOP);
    print_cfg_int(CONFIG_NB_ROUNDS);
    print_cfg_int(CONFIG_ITER_PER_ROUND);
    printf("--- >8 ---\n\n");
    fflush(stdout);

    if (sys_noop() != 0xC0FFEE) {
	    fprintf(stderr,
		    "I did not get any coffee when calling `noop` (%d), does your kernel support it?\n",
		    __NR_noop);
	    fflush(stderr);
	    exit(1);
    }

#if CONFIG_WARMUP
    /* We warm up in order to warm up involved caches */
    overhead_tsc[0]   = tsc_bench(CONFIG_ITER_PER_ROUND);
    overhead_scall[0] = syscall_bench(CONFIG_ITER_PER_ROUND);
    overhead_fcall[0] = funcall_bench(CONFIG_ITER_PER_ROUND);
#endif

    for (i = 0;
	 i < CONFIG_NB_ROUNDS;
	 i++) {
	    overhead_tsc[i]   = tsc_bench(CONFIG_ITER_PER_ROUND);
	    overhead_scall[i] = syscall_bench(CONFIG_ITER_PER_ROUND);
	    overhead_fcall[i] = funcall_bench(CONFIG_ITER_PER_ROUND);
    }


    /* ------------------- PRINT RESULT ------------------- */
    for (i = 0;
	 i < CONFIG_NB_ROUNDS;
	 i++) {
	    if (overhead_tsc[i])
		    printf(
#if CONFIG_BENCHTSC_AND_LOOP
			    "tsc+loop(%d)\t%"PRIu64"\n",
			    CONFIG_ITER_PER_ROUND,
			    overhead_tsc[i]
#else
			    "tsc\t%"PRIu64"\n",
			    overhead_tsc[i]
#endif
			    );

#if CONFIG_ITER_PER_ROUND >= 1000
	    printf("scall\t%"PRIu64".%03"PRIu64"\n",
		   (overhead_scall[i] - overhead_tsc[i]) / CONFIG_ITER_PER_ROUND,
		   (((overhead_scall[i] - overhead_tsc[i]) * 1000) / CONFIG_ITER_PER_ROUND) % 1000);
	    printf("fcall\t%"PRIu64".%03"PRIu64"\n\n",
		   (overhead_fcall[i] - overhead_tsc[i]) / CONFIG_ITER_PER_ROUND,
		   (((overhead_fcall[i] - overhead_tsc[i]) * 1000) / CONFIG_ITER_PER_ROUND) % 1000);
#else
	    printf("scall\t%"PRIu64"\n",
		   (overhead_scall[i] - overhead_tsc[i]) / CONFIG_ITER_PER_ROUND);
	    printf("fcall\t%"PRIu64"\n\n",
		   (overhead_fcall[i] - overhead_tsc[i]) / CONFIG_ITER_PER_ROUND);
#endif
    }

    fflush(stdout);
    fflush(stderr);
    sleep(1);
    return 0;
}
