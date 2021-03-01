#include <testsuite.h>
#include <test_assert.h>
#include <uk/essentials.h>
#include <uk/ctors.h>

static int __section(".zzzzz") __used test_var = 1;
static int __section(".test_ld_data") __used test_var1 = 1;

static void __ctor_test_var1(void)
{
	test_var1 = 20;
}
__UK_CTOR_FUNC(1, __ctor_test_var1);

static void __ctor_test_var(void)
{
	test_var = 10;
}
__UK_CTOR_FUNC(1, __ctor_test_var);

static void __ctor_test_var_dupl(void)
{
	test_var = 30;
}
__UK_CTOR_FUNC(2, __ctor_test_var_dupl);

static int test_lds(void)
{
	TEST_SNUM_EQUALS(test_var, 30);
	TEST_SNUM_EQUALS(test_var1, 20);
	return 0;
}
TESTSUITE_REGISTER(lds_test, test_lds);
