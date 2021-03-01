#include <uk/libparam.h>
#include <stdint.h>
#include <stdio.h>
#include <uk/arch/types.h>
#include <test_assert.h>
#include <testsuite.h>

#define TEST_UK_LIB_PARAM(name, type, defValue, pr_fmt, sign)		\
	static void print_ ## name (void)				\
	{								\
		printf("Expecting value:%"pr_fmt" defValue:%"pr_fmt"\n",\
			(type)name, (type)defValue);			\
		if (sign == 1)						\
			TEST_SNUM_EQUALS(defValue, name);		\
		else							\
			TEST_UNUM_EQUALS(defValue, name);		\
	}								\
	UK_LIB_PARAM(name, type);

#define TEST_STRING         "Hello TestSuite!"


static int test_var = 10;
static uint32_t test_var1 = 10;
static uint64_t test_var2 = 10;
static int64_t  test_var3 = 10;
static __u16 test_var4 = 10;
static __s16 test_var5 = 10;
static __u8 test_var6 = 10;
static char test_var7 = 'A';
static const char *testString = "Hello WOrld";
static __u64 testarray[] = {1,2,3,4,5};

static int testarr1[3];
static __u16 testarr2[3];
static __s64 testarr3[3];

TEST_UK_LIB_PARAM(test_var1, __u32, 40, __PRIu32, 0);
TEST_UK_LIB_PARAM(test_var, int, -30, __PRIs32, 1);
TEST_UK_LIB_PARAM(test_var2, __u64, 1024,__PRIu64, 0);
TEST_UK_LIB_PARAM(test_var3, __s64, -1024, __PRIs64,1);
TEST_UK_LIB_PARAM(test_var4, __u16, 0,__PRIu16, 0);
TEST_UK_LIB_PARAM(test_var5, __s16, 0,__PRIs16, 1);
TEST_UK_LIB_PARAM(test_var6, __u8, 128, __PRIu8, 0);
TEST_UK_LIB_PARAM(test_var7, char,'B', __PRIs8, 1);
UK_LIB_PARAM_STR(testString);
UK_LIB_PARAM_ARR(testarr1, int);
UK_LIB_PARAM_ARR(testarr2, __u16);
UK_LIB_PARAM_ARR(testarr3, __s64);

static void print_test_string()
{
	printf("String : %s\n", testString);
}

#define TEST_ARR_CHK(type, arr, size, sign)				\
	do {								\
		type *testptr = (type *)(arr);				\
		for ( j = 0; j < (size); j++) {				\
			printf("%llu -- %llu--%d\n", testptr[j], testarray[j],\
					size);				\
			if (sign)					\
				UK_ASSERT(testptr[j] == (type)testarray[j]);\
			else						\
				UK_ASSERT(testptr[j] == (type)testarray[j]);\
		}							\
	} while(0)

static void print_test_array()
{
	int j;

	TEST_ARR_CHK(int, testarr1,sizeof(testarr1)/sizeof(testarr1[0]),1);
	TEST_ARR_CHK(__u16, testarr2,sizeof(testarr2)/sizeof(testarr2[0]),0);
	TEST_ARR_CHK(__s64, testarr3,2,1);
}

static int test_lib_param(void)
{
	printf("Testvar4 %d\n", test_var4 & 0xffff);
	printf("Testvar5 %d\n", test_var5 & 0xffff);
	print_test_var();
	print_test_var1();
	print_test_var2();
	print_test_var3();
	print_test_var4();
	print_test_var5();
	print_test_var6();
	print_test_var7();
	print_test_string();
	print_test_array();
	return 0;
}
TESTSUITE_REGISTER(libuk_args, test_lib_param);
