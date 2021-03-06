#include <testsuite.h>
#include <test_assert.h>
#include <vfscore/mount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct vfscore_stat {
	int rc;
	int errcode;
	char *filename;
}vfscore_stat_t;


static vfscore_stat_t test_stats [] = {
	{.rc = 0, .errcode = 0, .filename="/foo/file.txt"},
	{.rc = -1, .errcode = EINVAL, .filename = NULL},
};


static int setup_vfscore_file(void)
{

	int ret = mount("", "/", "ramfs", 0, NULL);
	UK_ASSERT(ret == 0);
	UK_ASSERT(!mkdir("/foo", S_IRWXU));

	int fd = open("/foo/file.txt", O_WRONLY | O_CREAT, S_IRWXU);
	UK_ASSERT(fd > 2);
	int i;

	UK_ASSERT(
		write(fd, "hello world\n", sizeof("hello world\n"))
		== sizeof("hello world\n")
		);
	fsync(fd);
	return fd;
}


static void test_vfscore_stat(void)
{
	struct stat buf;
	int rc;
	int i;

	vfscore_stat_t *ptr = &test_stats[0];

	int fd = setup_vfscore_file();

	for ( i = 0; i < sizeof(test_stats) / sizeof(test_stats[0]); i++, ptr++) {
		errno = 0;
		rc = stat(ptr->filename, &buf);
		TEST_SNUM_EQUALS(rc, ptr->rc);
		rc = errno;
		TEST_SNUM_EQUALS(rc, ptr->errcode);
	}
	close(fd);
}

static int test_vfscore(void)
{
	test_vfscore_stat();
	return 0;
}
TESTSUITE_REGISTER(libvfscore, test_vfscore_stat);
