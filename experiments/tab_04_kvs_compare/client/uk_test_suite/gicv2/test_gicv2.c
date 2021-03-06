#include <uk/config.h>
#include <testsuite.h>
#include <test_assert.h>
#include <kvm/config.h>

static test_probe_gicv2(void)
{
	int rc;

	//rc = _dtb_init_gic(_libkvmplat_cfg.dtb);
	//TEST_ZERO_CHK(rc);
}

int test_gicv2(void)
{
	printf("Testing %s", __func__);
	test_probe_gicv2();
	return 0;
}
TESTSUITE_REGISTER(gicv2, test_gicv2);
