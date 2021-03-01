#include <testsuite.h>
#include <uk/assert.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

static void test_ioctl_fcntl(void)
{
	int fd = open("/foo/1.txt", O_RDWR);
	int rc, rc1;
	rc = fcntl(fd, F_SETFD, FD_CLOEXEC);
	rc = ioctl(fd, FIOCLEX);

	printf("%d -- %d \n", rc, rc1);
	return 0;
}

static int test_ioctl(void)
{
	int rc = mount("", "/", "ramfs", 0, NULL);
	printf("Mount retu %d", rc);
	UK_ASSERT(rc == 0);
	UK_ASSERT(!mkdir("/foo", S_IRWXU));
	int fd = open("/foo/1.txt", O_RDWR | O_CREAT, S_IRWXU);
	close(fd);

	test_ioctl_fcntl();
}
TESTSUITE_REGISTER(vfscore_test, test_ioctl);
