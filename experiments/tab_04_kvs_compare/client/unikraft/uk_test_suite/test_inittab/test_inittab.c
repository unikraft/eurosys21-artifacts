#include <uk/init.h>
#include <testsuite.h>
#include <test_assert.h>

static int test_early1(void)
{
	uk_pr_info("Test early init %s \n", __func__);
	return 0;
}
uk_early_initcall_prio(test_early1, 1);

static int test_early(void)
{
	uk_pr_info("Test early init %s \n", __func__);
	return 0;
}
uk_early_initcall(test_early);

static int test_plat1(void)
{
	uk_pr_info("Test plat init %s \n", __func__);
	return 0;
}
uk_plat_initcall_prio(test_plat1, 1);

static int test_plat(void)
{
	uk_pr_info("Test plat init %s \n", __func__);
	return 0;
}
uk_plat_initcall(test_plat);

static int test_lib1(void)
{
	uk_pr_info("Test lib init %s \n", __func__);
	return 0;
}
uk_lib_initcall_prio(test_lib1,2);

static int test_lib(void)
{
	uk_pr_info("Test lib init %s \n", __func__);
	return 0;
}
uk_lib_initcall(test_lib);

static int test_rootfs1(void)
{
	uk_pr_info("Test rootfs init %s \n", __func__);
	return 0;
}
uk_rootfs_initcall_prio(test_rootfs1, 3);

static int test_rootfs(void)
{
	uk_pr_info("Test rootfs init %s \n", __func__);
	return 0;
}
uk_rootfs_initcall(test_rootfs);

static int test_sys2(void)
{
	uk_pr_info("Test sys init %s \n", __func__);
	return 0;
}
uk_sys_initcall_prio(test_sys2, 4);

static int test_sys1(void)
{
	uk_pr_info("Test sys init %s \n", __func__);
	return 0;
}
uk_sys_initcall_prio(test_sys1, 4);

static int test_sys(void)
{
	uk_pr_info("Test early init %s \n", __func__);
	return 0;
}
uk_sys_initcall(test_sys);

static int test_late1(void)
{
	uk_pr_info("Test late init %s \n", __func__);
	return 0;
}
uk_late_initcall_prio(test_late1, 6);

static int test_late(void)
{
	uk_pr_info("Test late init %s \n", __func__);
	return 0;
}
uk_late_initcall(test_late);

static void test_cnt()
{
	uk_init_t *itr;
	int i = 0;

	uk_inittab_foreach(uk_inittab_start, uk_inittab_end, itr) {
		i++;
	}
	TEST_EXPR(i > 12);

	printf("%d\n", i);
}

static int test_inittab(void)
{
	test_cnt();
	return 0;
}
TESTSUITE_REGISTER(inittab, test_inittab);
