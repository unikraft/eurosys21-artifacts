#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <inttypes.h>
#include <string.h>
#include <errno.h>
#ifdef __Unikraft__
#include <shfs/shfs.h>
#include <shfs/shfs_fio.h>
#endif
#include "tsc.h"

#define TEST_COUNT 1000
#define TEST_REPEAT 1
#define TEST_NONE    "0000000000000000000000000000000000000000000000000000000000000000"
#define TEST_NULL    "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"
#define TEST_RND512  "7ece3dc37ee842c0d7e6f7a9e2831475f76c5f7aa1c0ff35bf385e1cd9c05499"
#define TEST_RND4K   "7e022976e3522c64794e5a5f2e74fa2cf7e107e99848a6b5f94c9ee9e88a86f1"
#define TEST_RND512K "a2d6d6755fb87165857c6519595f674819bb35f8e8b9dcf80cfdeca413275ea2"
#define TEST_RND4M   "55c7cbbb9892fc430eac034025b79b29e46811a8b205f2fbbeaba64d396b0c65"

#ifndef __Unikraft__
#define uk_pr_info(fmt, ...) fprintf(stdout, fmt, __VA_ARGS__)
#define uk_pr_warn(fmt, ...) fprintf(stdout, fmt, __VA_ARGS__)
#define uk_pr_err(fmt,  ...) fprintf(stderr, fmt, __VA_ARGS__)
#define uk_pr_crit(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__)

#else
extern int shfs_ls(void);
extern int shfs_info(void);

uint64_t test_shfs_open(const char *path, unsigned int count)
{
	uint64_t stime, etime;
	size_t i = 0;
	SHFS_FD f[count];
	unsigned int err = 0;

	/* warm-up */
	stime = bench_start();
	f[0] = shfs_fio_open(path);
	if (f[0])
		shfs_fio_close(f[0]);
	etime = bench_end();

	stime = bench_start();
	for (i = 0; i < count; ++i)
		f[i] = shfs_fio_open(path);
	etime = bench_end();

	for (i = 0; i < count; ++i) {
		if (f[i])
			shfs_fio_close(f[i]);
		else
			err++;
	}

	if (err)
		uk_pr_err("%u of %u descriptors were not opened\n",
			  err, count);

	return etime - stime;
}
#endif /* __Unikraft__ */

uint64_t test_vfs_open(const char *path, unsigned int count)
{
	uint64_t stime, etime;
	size_t i = 0;
	int f[count];
	unsigned int err = 0;

	/* warm-up */
	stime = bench_start();
	f[0] = open(path, O_RDONLY);
	if (f[0] >= 0)
		close(f[0]);
	else
		uk_pr_err("%s: %s\n", path, strerror(errno));
	etime = bench_end();

	stime = bench_start();
	for (i = 0; i < count; ++i)
		f[i] = open(path, O_RDONLY);
	etime = bench_end();

	for (i = 0; i < count; ++i) {
		if (f[i] >= 0)
			close(f[i]);
		else
			err++;
	}

	if (err)
		uk_pr_err("%u of %u descriptors were not opened\n",
			  err, count);

	return etime - stime;
}

int main(int argc, char *argv[])
{
	int ret = 0;
	unsigned int repeat = TEST_REPEAT;
	uint64_t bt;

#ifdef __Unikraft__
	repeat = TEST_REPEAT;

	shfs_info();
	shfs_ls();

	while (repeat-- > 0) {
		bt = test_shfs_open("?" TEST_NONE, TEST_COUNT);
		printf("SHFS,<n/a>,iterations:%u,avg:%"PRIu64".%"PRIu64"\n\n",
		       TEST_COUNT, bt / TEST_COUNT, (((bt * 1000) / TEST_COUNT) % 1000));

		bt = test_shfs_open("?" TEST_RND4K, TEST_COUNT);
		printf("SHFS,/rnd4k,iterations:%u,avg:%"PRIu64".%"PRIu64"\n\n",
		       TEST_COUNT, bt / TEST_COUNT, (((bt * 1000) / TEST_COUNT) % 1000));
	}
#endif /* __Unikraft__ */

	repeat = TEST_REPEAT;
	while (repeat-- > 0) {
		bt = test_vfs_open("/" TEST_NONE, TEST_COUNT);
		printf("VFS,<n/a>,iterations:%u,avg:%"PRIu64".%"PRIu64"\n\n",
		       TEST_COUNT, bt / TEST_COUNT, (((bt * 1000) / TEST_COUNT) % 1000));

		bt = test_vfs_open("/" TEST_RND4K, TEST_COUNT);
		printf("VFS,/rnd4k,iterations:%u,avg:%"PRIu64".%"PRIu64"\n\n",
		       TEST_COUNT, bt / TEST_COUNT, (((bt * 1000) / TEST_COUNT) % 1000));
	}

	fflush(stdout);
	fflush(stderr);
	sleep(1);
	return ret;
}
