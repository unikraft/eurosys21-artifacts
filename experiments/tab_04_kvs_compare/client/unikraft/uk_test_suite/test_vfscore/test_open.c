#include <testsuite.h>
#include <test_assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stddef.h>
#include <limits.h>
#include <errno.h>

#define VFSCORE_MAX_OPEN_FILE 1021

#define DEF_DIR		"/foo"

static int test_large_open(void)
{
	int i = 0;
	char filename[50];
	char readbuffer[50];
	int fd[VFSCORE_MAX_OPEN_FILE];
	int cnt[VFSCORE_MAX_OPEN_FILE];
	int rc = 0, tcnt;

	for (i = 0; i < VFSCORE_MAX_OPEN_FILE; i++) {
		tcnt = snprintf(filename, sizeof(filename), DEF_DIR"/%d.txt", i);
		rc = open(filename, O_WRONLY | O_CREAT, S_IRWXU);
		if (rc < 0) {
			printf("%s %d - %s\n", filename, errno, strerror(errno));
		}
		TEST_EXPR(rc > 2);
		close(rc);
	}

	for (i = 0; i < VFSCORE_MAX_OPEN_FILE; i++) {
		tcnt = snprintf(filename, sizeof(filename), DEF_DIR"/%d.txt", i);
		rc = open(filename, O_RDWR, S_IRWXU);
		TEST_EXPR(rc > 2);
		fd[i] = rc;
		rc = write(fd[i], filename, tcnt);
		TEST_SNUM_EQUALS(tcnt, rc);
		cnt[i] = tcnt;
	}

	for (i = 0; i < VFSCORE_MAX_OPEN_FILE; i++) {
		close(fd[i]);
	}

	for (i = 0; i < VFSCORE_MAX_OPEN_FILE; i++) {
		tcnt = snprintf(filename, sizeof(filename), DEF_DIR"/%d.txt", i);
		rc = open(filename, O_RDWR, S_IRWXU);
		TEST_EXPR(rc > 2);
		TEST_SNUM_EQUALS(rc,fd[i]);
		memset(readbuffer, 0, sizeof(readbuffer));
		rc = read(rc, readbuffer, sizeof(readbuffer));
		if (rc < 0) {
			printf("%d - %s\n", errno, strerror(errno));
		} else {
			printf("Readbuffer: %s,%s\n",readbuffer, filename);
		}
		TEST_SNUM_EQUALS(rc, cnt[i]);
		TEST_BYTES_EQUALS(filename, readbuffer, cnt[i]);
	}

	for (i = 0; i < VFSCORE_MAX_OPEN_FILE; i++) {
		close(fd[i]);
	}
}

static int test_fopen(void)
{
	int rc = mount("", "/", "ramfs", 0, NULL);

	UK_ASSERT(rc == 0);
	UK_ASSERT(!mkdir("/foo", S_IRWXU));

	test_large_open();
}
TESTSUITE_REGISTER(vfscore_test, test_fopen);
