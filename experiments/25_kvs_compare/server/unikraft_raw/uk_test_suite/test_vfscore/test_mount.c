#include <stddef.h>
#include <testsuite.h>
#include <test_assert.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

static void test_multimount(void)
{
	int rc;
	char buffer[10];

	rc = mkdir("/sys", S_IRWXU);
	TEST_SNUM_EQUALS(rc, 0);

	rc = mount("", "/sys", "ramfs", 0, NULL);

	TEST_SNUM_EQUALS(rc, 0);

#ifdef CONFIG_LIBDEVFS_AUTOMOUNT
	rc = mkdir("/dev", S_IRWXU);
	TEST_SNUM_EQUALS(rc, -1);

	rc = mount("", "/dev", "devfs", 0, NULL);
	TEST_EXPR(rc != 0);
#else
	rc = mkdir("/dev", S_IRWXU);
	TEST_SNUM_EQUALS(rc, 0);

	rc = mount("", "/dev", "devfs", 0, NULL);
	TEST_SNUM_EQUALS(rc, 0);

#endif /* CONFIG_LIBDEVFS_AUTOMOUNT */

	rc = mkdir("/tmp", S_IRWXU);
	TEST_SNUM_EQUALS(rc, 0);

	rc = mount("", "/tmp", "naivefs", 0, NULL);
	TEST_SNUM_EQUALS(rc, 0);

	rc = mount("", "/tmp", "naivetmpfs", 0, NULL);
	printf("Returne %d\n", rc);
	TEST_EXPR(rc != 0);
	//TEST_SNUM_EQUALS(errno, ENODEV);


	rc = open("/dev/random", O_RDONLY);
	TEST_EXPR(rc >= 0);

	rc = read(rc, &buffer, sizeof(buffer));
	TEST_EXPR(rc > 0);

	printf("%d\n", rc);
}

static int test_mount(void)
{
	test_multimount();
	return 0;
}
TESTSUITE_REGISTER(vfscore, test_mount);
