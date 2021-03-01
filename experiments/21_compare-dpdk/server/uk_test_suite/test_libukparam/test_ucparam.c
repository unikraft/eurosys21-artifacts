#include <testsuite.h>
#include <test_assert.h>
#define CONFIG_LIBUKLIBPARAM
#include "param.c"

#define PARAMS	15
#define PARAMPERARG  3

struct test_kernel_input_args {
	char* parameter[PARAMS];
	int iteration;
	int arg_cnt;
	int parg_cnt[PARAMS/PARAMPERARG];
	int rc[PARAMS/PARAMPERARG];
	int result[PARAMS/PARAMPERARG];
	unsigned int param_idx[PARAMS/PARAMPERARG];
	unsigned int  param_len[PARAMS/PARAMPERARG];
	unsigned int value_idx[PARAMS/PARAMPERARG];
	unsigned int value_len[PARAMS/PARAMPERARG];
};

#define MAX_SECTIONS     5
#define MAX_PARAM_SIZE   40

struct test_param_args {
	char parameter[MAX_PARAM_SIZE];
	int param_idx;
	int param_len;
	int value_idx;
	int value_len;
	int lib_len;
};

struct test_library_search {
	struct uk_lib_section libsections[MAX_SECTIONS];
	struct test_param_args args;
	int rc;
	int section_idx;
	uint32_t lib_len;
	int cnt;
};

struct test_args_santize {
	char value[50];
	int value_length;
	int rc;
	int svalue_idx;
	int evalue_idx;
	int value_len;
};

struct test_args_parse {
	struct test_param_args args;
	int rc;
	int param_idx;
};

#define MAX_VALUE_LEN            20
struct test_value_set {
	/* input parameter */
	char param[MAX_VALUE_LEN];
	int param_type;
	int size;
	int sign;
	int scopy;
	int base;
	int rc;
	union {
		char cvalue;
		__u8 bvalue;
		__s16 ssvalue;
		__u16 usvalue;
		int   value;
		unsigned uvalue;
		unsigned long long ulvalue;
		signed long long slvalue;
	} value;
	void *address;
};

struct test_ptr {
	uintptr_t ptr;
	int len;
};

struct test_value_sets {
	struct test_param_args test_args;
	int param_idx;
	int rc;
	int sign;
	int scopy;
	int type_size;
	union {
		char cvalue;
		__u8 bvalue;
		__s16 ssvalue;
		__u16 usvalue;
		int   value;
		unsigned uvalue;
		unsigned long long ulvalue;
		signed long long slvalue;
		uintptr_t addr;
	} value;
	uintptr_t address;
	int size;
};

#define test_param(name, type,value)					\
	static __used const char *name ## _apptestsuite = #name;	\
	static type name = value


#define test_param_arr(name, type, cnt)					\
	static __used const char *name ## _apptestsuite = #name;	\
	static type name[cnt] = {0};

test_param(testchar,char,'A');
test_param(test,int,20);
test_param(test1,uint32_t,20);
test_param(test12,uint64_t,20);
test_param(test132,int64_t,20);
test_param(test123,uint16_t,20);
test_param(var,int16_t,20);
test_param(overflow,int16_t,20);
test_param(ochar,char,'B');
#ifdef CONFIG_TESTSTRINGPARAM
test_param(teststring,const char *,"Hello World");
#endif /* CONFIG_TESTSTRINGPARAM */
#ifdef CONFIG_TESTARRAYPARAM
test_param_arr(testarr1, __u32, 5);
test_param_arr(testarr2, int, 10);
test_param_arr(testarr3, __u64, 2);
#endif /* CONFIG_TESTARRAYPARAM */

static __u64 testarray[] = {1,2,3,4,5};

static __attribute__ ((section("apptestsuite__param_arg"),used))
	struct uk_param test_params[] = {
	{.name="apptestsuite.testchar", .param_type=_LIB_PARAM___s8,
	 .param_size=1, .addr=(uintptr_t) &testchar},
	{.name="apptestsuite.test", .param_type=_LIB_PARAM___s32,
	 .param_size=1, .addr=(uintptr_t) &test},
	{.name="apptestsuite.test1", .param_type=_LIB_PARAM___u32,
	 .param_size=1,.addr=(uintptr_t)&test1},
	{.name="apptestsuite.test12", .param_type=_LIB_PARAM___u64,
	 .param_size=1,.addr=(uintptr_t)&test12},
	{.name="apptestsuite.test132", .param_type=_LIB_PARAM___s64,
	 .param_size=1, .addr=(uintptr_t)&test132},
	{.name="apptestsuite.test123", .param_type=_LIB_PARAM___u16,
	  .param_size=1,.addr=(uintptr_t)&test123},
	{.name="apptestsuite.var", .param_type=_LIB_PARAM___s16,
	 .param_size=1,.addr=(uintptr_t)&var},
#ifdef CONFIG_TESTSTRINGPARAM
	{.name="apptestsuite.testString", .param_type=_LIB_PARAM___uptr,
	 .param_size=1,.addr=(uintptr_t)&teststring},
#endif /* CONFIG_TESTSTRINGPARAM */
#ifdef CONFIG_TESTARRAYPARAM
	{.name="apptestsuite.testarr1", .param_type=_LIB_PARAM___u32,
	 .param_size=5,.addr=(uintptr_t)&testarr1[0]},
	{.name="apptestsuite.testarr2", .param_type=_LIB_PARAM_int,
	 .param_size=10,.addr=(uintptr_t)&testarr2[0]},
	{.name="apptestsuite.testarr3", .param_type=_LIB_PARAM___u64,
	 .param_size=2,.addr=(uintptr_t)&testarr3[0]},
#endif /* CONFIG_TESTARRAYPARAM */
};

struct test_value_sets values_param[] = {
	/* Test a char */
	{.test_args={.parameter="apptestsuite.testchar=B", .param_len=21,
	.value_idx=22, .value_len=1,.lib_len=12}, .param_idx=0, .sign=1,
	.scopy = 0, .rc = 0, .type_size = sizeof(testchar),
	.value.cvalue='B', .address=(uintptr_t)&testchar, .size=1},

	/* Test an int */
	{.test_args={.parameter="apptestsuite.test=-20", .param_len=17,
	.value_idx=18, .value_len=1},.param_idx=1, .sign=1, .scopy=0,
	.type_size=sizeof(test), .value.value=-20, .rc=0,
	.address=(uintptr_t)&test, .size=1},

	/* Test an uint64_t */
	{.test_args={.parameter="apptestsuite.test12=18446744073709551615",
	 .param_len=19, .value_idx=20, .value_len=20},.param_idx=3, .rc=0,
	 .value.ulvalue=0xFFFFFFFFFFFFFFFF, .address=(uintptr_t)&test12,
	 .size=1},

	/* Test an int64_t */
	{.test_args={.parameter="apptestsuite.test132=9223372036854775807",
	 .param_len=20, .value_idx=21, .value_len=19}, .param_idx=4, .rc=0,
	 .scopy = 0, .sign = 0, .type_size=sizeof(test132),
	 .value.slvalue=0x7FFFFFFFFFFFFFFF, .address=(uintptr_t)&test132,
	 .size=1},

	/* Test a uint16_t */
	{.test_args={.parameter="apptestsuite.test123=300",
	 .param_len=20, .value_idx=21, .value_len=3}, .param_idx=5, .rc =0,
	 .scopy = 0, .sign = 0, .type_size=sizeof(test132),
	 .value.slvalue=300, .address=(uintptr_t)&test123,
	 .size=1},

#ifdef CONFIG_TESTSTRINGPARAM
	/* Test a string */
	{.test_args={.parameter="apptestsuite.teststring=Hello Test",
	 .param_len=23, .value_idx=24, .value_len=10},.param_idx=7, .rc=0,
	 .scopy = 1, .sign = 1, .type_size=1,
	 .value.addr=(uintptr_t)&teststring,
	 .address=(uintptr_t)&teststring,
	 .size=10},
#endif /* CONFIG_TESTSTRINGPARAM */
#ifdef CONFIG_TESTARRAYPARAM

	/* Test an entire array */
	{.test_args={.parameter="apptestsuite.testarr1=1 2 3 4 5",
	 .param_len=21, .value_idx=22, .value_len=11},.param_idx=8, .rc=0,
	 .scopy = 0, .sign = 0, .type_size=4,
	 .value.addr=(uintptr_t)&testarr1[0], .address=(uintptr_t)&testarr1[0],
	 .size=5},

	/* Test an partial array */
	{.test_args={.parameter="apptestsuite.testarr2=1 2 3",
	 .param_len=21, .value_idx=22, .value_len=5},.param_idx=9, .rc=0,
	 .scopy = 0, .sign = 1, .type_size=4,
	 .value.addr=(uintptr_t)&testarr2[0], .address=(uintptr_t)&testarr2[0],
	 .size=3},

	/* Overflow test */
	{.test_args={.parameter="apptestsuite.testarr3=1 2 3",
	 .param_len=21, .value_idx=22, .value_len=5},.param_idx=10, .rc=0,
	 .scopy = 0, .sign = 0, .type_size=8,
	 .value.addr=(uintptr_t)&testarr3[0], .address=(uintptr_t)&testarr3[0],
	 .size=2},
#endif /* CONFIG_TESTARRAYPARAM */
};

struct test_value_set value_param[] = {
	/* Test for the different variable type */
	/* int */
	{.param="-30",.param_type= _LIB_PARAM___s32,.size=sizeof(int),
	 .sign=1,.scopy=0, .rc=0,.address=&test, .value.value=-30},
	/* unsigned */
	{.param="4294967295", .param_type=_LIB_PARAM___u32,
	  .size=sizeof(uint32_t),.sign=0,.scopy=0,
	 .rc=0,.address=&test1, .value.uvalue=0xFFFFFFFF},
	/* unsigned long */
	{.param="18446744073709551615",.param_type=_LIB_PARAM___u64,
	 .size=sizeof(uint64_t),.sign=0,.scopy=0,
	 .rc=0,.address=&test12, .value.ulvalue=0xFFFFFFFFFFFFFFFF},
	/* long */
	{.param="9223372036854775807",.size=sizeof(int64_t),.sign=1,.scopy=0,
	 .rc=0,.address=&test132,.value.slvalue=0x7FFFFFFFFFFFFFFF,
	 .param_type=_LIB_PARAM___s64},

	/* Char */
	{.param="C",.size=sizeof(char),.sign=1,.scopy=0,
	 .rc=0,.address=&testchar,.value.cvalue='C',
	 .param_type=_LIB_PARAM___s8},

	/* Test for invalid type of parameter */
	/* Invalid sign */
	{.param="30", .size=0, .scopy=0, .sign=8, .rc=-EINVAL,.address=&var,
	 .value.ssvalue=20},

	/* Invalid size */
	{.param="30", .size=16, .scopy=0, .sign=0, .rc=-EINVAL,.address=&var,
	 .value.ssvalue=20},

	/* Test for invalid value */
	/* NULL address */
	{.param="30", .size=4, .scopy=0, .sign=0, .rc=-EINVAL,.address=NULL,
	 .value.value=20},

	/* Detect number Overflow/Underflow */
	{.param="4294967295", .size=2, .scopy=0, .sign=1, .rc=0,
	 .address=&overflow, .value.usvalue=0xFFFF},

	/* Detect character overflow */
	{.param="RC", .size=1, .scopy=0, .sign=1, .rc=0,
	 .address=&ochar, .value.cvalue='R'},
};


static struct test_args_parse args_parse[] = {
	/* Select a first idx */
	{.args = {.parameter="apptestsuite.test=2", .param_idx =0,
		  .param_len=17,.value_idx=18,.value_len=1},
	 .rc=0,.param_idx=1},

	/* Select a idx in the middle */
	{.args = {.parameter="apptestsuite.test12=40", .param_idx =0,
		  .param_len=19,.value_idx=20,.value_len=2},
	 .rc=0,.param_idx=3},

	/* Select a non existing value */
	{.args = {.parameter="apptestsuite.i=40", .param_idx =0,
		  .param_len=14,.value_idx=15,.value_len=2},
	 .rc=-EINVAL,.param_idx=-1},

	/* Select a non existing similar prefix */
	{.args = {.parameter="apptestsuite.test1234=60", .param_idx =0,
		  .param_len=21,.value_idx=22,.value_len=2},
	 .rc=-EINVAL,.param_idx=-1},
};

static struct test_kernel_input_args ip_args[] = {
	{ .parameter= {"test.value1=","value1"}, .iteration = 1,.arg_cnt=2,
	  .parg_cnt={2}, .result={2}, .rc={0},.param_idx={0}, .param_len={11},
	  .value_idx={12}, .value_len={6}},
	{ .parameter={"test.value1","=","value1"}, .iteration = 1, .arg_cnt=3,
	  .parg_cnt={3}, .result={3}, .rc={0}, .param_idx={0}, .param_len={11},
	  .value_idx={12}, .value_len={6}},
	{ .parameter={"test.value1=value1"}, .iteration=1, .arg_cnt=1,
	  .parg_cnt={1}, .param_idx={0}, .result={1},.rc={0}, .param_len={11},
	  .value_idx={12}, .value_len={6}},
	{ .parameter={"test.value1","=value1"}, .iteration=1, .arg_cnt=2,
	  .parg_cnt={1}, .param_idx={0}, .result={2},.rc={0}, .param_len={11},
	  .value_idx={12}, .value_len={6} },
	{ .parameter={"test.value1 value1"}, .iteration=1, .arg_cnt=1,
	  .parg_cnt={1}, .param_idx={0}, .result={1}, .rc={-EINVAL},
	  .param_len={18}, .value_idx={0}, .value_len={0} },
	{ .parameter={"test.value1="}, .iteration=1, .arg_cnt=1, .param_idx={0},
	  .parg_cnt={1}, .result={1}, .rc={-EINVAL}, .param_len={11},
	  .value_idx={0}, .value_len={0} },
	{ .parameter={"test.value1","="}, .iteration=1, .arg_cnt=2,
	  .param_idx= {0}, .parg_cnt={1},.result={2}, .rc={-EINVAL},
	  .param_len={11}, .value_idx={0},.value_len={0} },
	{ .parameter={"test.value1=","="}, .iteration=1,.arg_cnt=2,
	  .param_idx={0}, .parg_cnt={1}, .result={2}, .rc={-EINVAL},
	  .param_len={11}, .value_idx={0}, .value_len={0} },
	{ .parameter={"test.value1=","="}, .iteration=1, .arg_cnt=2,
	  .param_idx={0},.parg_cnt={2}, .result={2}, .rc={-EINVAL},
	  .param_len={11},.value_idx={0},.value_len={0} },
	{ .parameter={"test.value1","=", "test.value2=","23"}, .iteration=2,
	  .arg_cnt=4, .parg_cnt={3,2}, .result={3,1},
	  .rc={-EINVAL, -EINVAL},.param_idx={0,0}, .param_len={11,2},
	  .value_idx={0,0}, .value_len={0,0} },
	{ .parameter={"test.val=ue1="}, .iteration=1, .arg_cnt=1,
	  .parg_cnt = {1}, .result={1}, .rc={-EINVAL}, .param_idx={0},
	  .param_len={0}, .value_idx={0}, .value_len={0} },
};

static struct test_library_search lib_src[] = {
	/** Value Found **/
	{.libsections = {{.lib_name="apptestsuite4", .len=10},
			{.lib_name="apptestsuite1", .len=20},
			{.lib_name="apptestsuite2", .len=30},
			{.lib_name="apptestsuite3", .len=40}}, .cnt = 4,
			.args = {.parameter="apptestsuite4.test=10",
				 .param_idx = 0,
				.param_len=18, .value_idx =19,.value_len = 2},
			.rc = 0,.lib_len=13, .section_idx=0,
	},
	/** Value Found in a middle of the list**/
	{.libsections = {{.lib_name="apptestsuite", .len=10},
			{.lib_name="apptestsuite1", .len=20},
			{.lib_name="apptestsuite2", .len=30},
			{.lib_name="apptestsuite3", .len=40}}, .cnt = 4,
			.args = {.parameter="apptestsuite2.test=10",
				 .param_idx = 0,
				.param_len=18, .value_idx =19,.value_len = 2},
			.rc = 0,.lib_len=13, .section_idx=2,
	},
	/** Section Not found **/
	{.libsections ={{.lib_name="apptestsuite", .len=10},
			{.lib_name="apptestsuite1", .len=20},
			{.lib_name="apptestsuite2", .len=30},
			{.lib_name="apptestsuite3", .len=40}}, .cnt = 4,
			.args = {.parameter="suite.test=10", .param_idx = 0,
				.param_len=10, .value_idx =12,.value_len = 2},
			.rc = -EINVAL,.lib_len=0, .section_idx=-1,
	},
	/** Section with similar prefix **/
	{.libsections ={{.lib_name="apptestsuite", .len=10},
			{.lib_name="apptestsuite1", .len=20},
			{.lib_name="apptestsuite2", .len=30},
			{.lib_name="apptestsuite3", .len=40}}, .cnt = 4,
			.args = {.parameter="apptestsuite21.test=10",
				 .param_idx = 0,
				.param_len=19, .value_idx =20,.value_len = 2},
			.rc = -EINVAL,.lib_len=0, .section_idx=-1,
	},
};

static struct test_args_santize test_santize[] = {
	{ .value="\"test\"", .value_length=6, .rc = 0, .svalue_idx=1,
	 .evalue_idx=5, .value_len = 4},
	{ .value="   \"test\"", .value_length=9, .rc = 0,
	 .svalue_idx=4,.evalue_idx=8, .value_len = 4},
	{ .value="  \"test\" ", .value_length=9, .rc = 0,
	 .svalue_idx=3,.evalue_idx=7, .value_len = 4},
	{ .value="\"test\"    ", .value_length=10, .rc = 0,
	 .svalue_idx=1,.evalue_idx=5, .value_len = 4},
	{ .value="test\"    ", .value_length=9, .rc = -EINVAL,
	 .svalue_idx=0,.evalue_idx=9, .value_len = 4},
	{ .value="test", .value_length=4, .rc = 0,
	 .svalue_idx=0,.evalue_idx=4, .value_len = 4},
	{ .value="test\"\"    ", .value_length=10, .rc = -EINVAL,
	 .svalue_idx=0,.evalue_idx=10, .value_len = 4},
	{ .value="test\"\"    ", .value_length=10, .rc = -EINVAL,
	 .svalue_idx=0,.evalue_idx=10, .value_len = 4},
};

static int test_kernel_arg_fetch(void)
{
	int i = 0;
	int j = 0;
	int len = sizeof(ip_args) / sizeof(ip_args[0]);
	int rc = 0;
	struct param_args pargs = {0};
	int acnt = 0;

	for (i = 0; i < len; i++) {
		int curr_idx = 0;
		for (j = 0; j < ip_args[i].iteration; j++) {
			printf("id: %d parameter:%s\n", i,
				ip_args[i].parameter[curr_idx]);
			rc = kernel_arg_fetch(&ip_args[i].parameter[curr_idx],
					      ip_args[i].arg_cnt - curr_idx,
					      &pargs, &acnt);
			printf("pargs param_len: %d value_len: %d args: %d\n",
					pargs.param_len, pargs.value_len, acnt);
			/* check for the return code */
			TEST_SNUM_EQUALS(rc, ip_args[i].rc[j]);
			/* Check for the number of arguments processed */
			TEST_SNUM_EQUALS(acnt, ip_args[i].result[j]);


			/* Check for the parameter index */
			int k = 0;
			unsigned int plen = 0;
			unsigned int pplen = 0;
			/* Check for the parameter length */
			TEST_SNUM_EQUALS(pargs.param_len,
					ip_args[i].param_len[j]);
			if (ip_args[i].param_len[j]) {
				/* Check for parameter pointer */
				do {
					pplen = strlen(
					    ip_args[i].parameter[curr_idx + k]);
					if (plen + pplen >
						ip_args[i].param_idx[j]) {
						TEST_PTR_EQUALS(
						ip_args[i].parameter[curr_idx+k] +
							(ip_args[i].param_idx[j] - plen),
							pargs.param);
						break;
					}
					k++;
					plen += pplen;
				} while (k < acnt);
			} else {
				TEST_NULL(pargs.value);
				TEST_ZERO_CHK(pargs.value_len);
			}
			k = 0;
			plen = 0;
			if (ip_args[i].value_len[j]) {
				TEST_UNUM_EQUALS(pargs.value_len,
						 ip_args[i].value_len[j]);
				do {
					pplen = strlen(ip_args[i].parameter[
							curr_idx + k]);
					/* Check for the value index */
					if (plen + pplen >
						ip_args[i].value_idx[j]) {
						TEST_PTR_EQUALS(
						(ip_args[i].parameter[curr_idx + k]
						+ (ip_args[i].value_idx[j] - plen)),
							pargs.value);
					}
					k++;
					plen += pplen;
				} while (k < acnt);
			} else {
				TEST_NULL(pargs.value);
				TEST_ZERO_CHK(pargs.value_len);
			}
			/* Check for the value length */
			curr_idx += ip_args[i].result[j];
		}
		printf("test :%d completed\n", i);
	}

	return 0;
}

static void setup_uk_section(struct uk_lib_section *sec, int size)
{
	int i = 0;

	while (i < size) {
		uk_pr_info("Adding section %d@%p\n", i, sec);
		_uk_libparam_lib_add(sec++);
		i++;
	}
}

static void cleanup_uk_section(struct uk_lib_section *sec, int size)
{
	int i = 0;

	while (i < size) {
		uk_list_del(&sec->next);
		sec++;
		i++;
	}
}

static void setup_param_args(struct test_param_args *args,
			     struct param_args *pargs)
{
	pargs->param = &args->parameter[0] + args->param_idx;
	pargs->param_len = args->param_len;

	pargs->value = &args->parameter[0] + args->value_idx;
	pargs->value_len = args->value_len;

	pargs->lib_len = args->lib_len;
}

static int test_libsearch(void)
{
	int i, rc;
	int len = sizeof(lib_src) / sizeof(lib_src[0]);
	struct param_args pargs = {0};
	struct uk_lib_section *libsections;

	for (i = 0; i < len; i++) {
		setup_uk_section(lib_src[i].libsections, lib_src[i].cnt);
		setup_param_args(&lib_src[i].args, &pargs);
		rc = kernel_lib_fetch(&pargs, &libsections);

		TEST_SNUM_EQUALS(rc, lib_src[i].rc);

		if (lib_src[i].section_idx >= 0) {
			uk_pr_info("%p --- %p %d %s\n", libsections,
			&lib_src[i].libsections[lib_src[i].section_idx],
			lib_src[i].section_idx, (libsections)->lib_name);
			TEST_PTR_EQUALS(libsections,
					&lib_src[i].libsections[lib_src[i].section_idx]);
			TEST_UNUM_EQUALS(pargs.lib_len, lib_src[i].lib_len);
		} else {
			TEST_ZERO_CHK(lib_src[i].lib_len);
		}
		cleanup_uk_section(&lib_src[i].libsections[0], lib_src[i].cnt);

	}

	return 0;
}


static int test_kernel_parse_arg(void)
{
	int i, rc;
	int len = sizeof(args_parse) / sizeof(args_parse[0]);
	struct uk_lib_section *libsections;
	struct uk_param *param;
	struct param_args pargs = {0};
/**
 *  struct test_param_args args;
 *  int rc;
 *  int param_idx;
 */
	for (i = 0; i < len; i++) {
		setup_param_args(&args_parse[i].args, &pargs);
		rc = kernel_lib_fetch(&pargs, &libsections);
		TEST_ZERO_CHK(rc);
		uk_pr_info("param_len: %d, library_len: %d\n", pargs.param_len,
				pargs.lib_len);
		rc = kernel_parse_arg(&pargs, libsections, &param);
		uk_pr_info("rc: %d and args-rc: %d", rc, args_parse[i].rc);
		TEST_SNUM_EQUALS(rc, args_parse[i].rc);

		if (rc == 0) {
			TEST_PTR_EQUALS(param,
					&test_params[args_parse[i].param_idx]);
		}
	}

	return 0;
}

#define TEST_ARR_CHK(type, arr, size, sign)				\
	do {								\
		type *testptr = (type *)(arr);				\
		for ( j = 0; j < size; j++) {				\
			printf("%llu -- %llu\n", testptr[j], testarray[j]);\
			if (sign)					\
				UK_ASSERT(testptr[j] == (type)testarray[j]);\
			else						\
				UK_ASSERT(testptr[j] == (type)testarray[j]);\
		}							\
	} while(0)

static int test_args_set(void)
{
	int len, rc, i, j;
	struct param_args pargs = {0};

	len = sizeof(values_param) / sizeof(values_param[0]);
	for (i = 0; i < len; i++) {
		memset(&pargs, 0, sizeof(pargs));
		setup_param_args(&values_param[i].test_args, &pargs);
		rc = kernel_args_set(&pargs,
				     &test_params[values_param[i].param_idx]);

		TEST_SNUM_EQUALS(values_param[i].rc, rc);
		if (rc == 0) {
			if (values_param[i].scopy == 1) {
				/* */
				TEST_PTR_EQUALS(*(char **)values_param[i].value.addr,
						*(char **)values_param[i].address);
				TEST_BYTES_EQUALS(
					*(char **)values_param[i].value.addr,
					*(char **)values_param[i].address,
						 values_param[i].size);
			} else if (values_param[i].size > 1) {
				switch(values_param[i].type_size) {
				case 1:
					if (values_param[i].sign) {
						TEST_ARR_CHK(char,
						(char *)values_param[i].address,
						values_param[i].size,
						values_param[i].sign);
					}
					else {
						TEST_ARR_CHK(__u8,
						(__u8 *)values_param[i].address,
						values_param[i].size,
						values_param[i].sign);
					}
					break;
				case 2:
					if (values_param[i].sign) {
						TEST_ARR_CHK(__s16,
						(__s16 *)values_param[i].address,
						values_param[i].size,
						values_param[i].sign);
					} else {
						TEST_ARR_CHK(__u16,
						(__u16 *)values_param[i].address,
						values_param[i].size,
						values_param[i].sign);
					}
					break;
				case 4:
					if (values_param[i].sign) {
						TEST_ARR_CHK(__s32,
						(__s32 *)values_param[i].address,
						values_param[i].size,
						values_param[i].sign);
					} else {
						TEST_ARR_CHK(__u32,
						(__u32 *)values_param[i].address,
						values_param[i].size,
						values_param[i].sign);
					}
					break;
				case 8:
					if (values_param[i].sign) {
						TEST_ARR_CHK(__s64,
						(__s64 *)values_param[i].address,
						values_param[i].size,
						values_param[i].sign);
					} else {
						TEST_ARR_CHK(__u64,
						(__u64 *)values_param[i].address,
						values_param[i].size,
						values_param[i].sign);
					}
					break;
				}
			} else if (values_param[i].sign) {
				TEST_BYTES_EQUALS(
					(void *)values_param[i].address,
						  &values_param[i].value.slvalue,
						  values_param[i].type_size);
			} else {
				TEST_BYTES_EQUALS(
					(void *)values_param[i].address,
						  &values_param[i].value.ulvalue,
						  values_param[i].size);
			}
		}
	}
	return 0;
}

int test_value_set(void)
{
	int len, rc, i;

	len = sizeof(value_param) / sizeof(value_param[0]);
	for (i = 0; i < len; i++) {
		rc = kernel_arg_set(value_param[i].address,
				    value_param[i].param,
				    value_param[i].size,
				    value_param[i].sign);

		TEST_SNUM_EQUALS(value_param[i].rc, rc);
		if (rc == 0) {
			if (value_param[i].sign) {
				TEST_BYTES_EQUALS(value_param[i].address,
						  &value_param[i].value.slvalue,
						  value_param[i].size);
			} else {
				TEST_BYTES_EQUALS(value_param[i].address,
						  &value_param[i].value.ulvalue,
						  value_param[i].size);
			}
		}
	}
	return 0;
}

static int test_argument_sanitize(void)
{

	int i = 0;
	int len = sizeof(test_santize) / sizeof(test_santize[0]);
	int rc = 0;
	struct param_args param;

	for (i = 0; i < len; i++) {
		param.value = test_santize[i].value;
		param.value_len =  test_santize[i].value_length;
		rc = kernel_value_sanitize(&param);
		uk_pr_debug("%p -- %p %d\n",
			    param.value, test_santize[i].value, param.value_len);

		uk_pr_debug("%p -- %p %d\n",
			    param.value + param.value_len,
			    test_santize[i].value + test_santize[i].evalue_idx,
			    param.value_len);
		TEST_EXPR(rc == test_santize[i].rc);
		TEST_EXPR(test_santize[i].value + test_santize[i].svalue_idx ==
				param.value);
		TEST_EXPR(test_santize[i].value + test_santize[i].evalue_idx ==
				param.value + param.value_len);
	}
	return 0;
}

int test_libargs_uc(void)
{
	int rc = 0;

	rc = test_kernel_arg_fetch();
	TEST_ZERO_CHK(rc);
	rc = test_libsearch();
	TEST_ZERO_CHK(rc);
	rc = test_kernel_parse_arg();
	TEST_ZERO_CHK(rc);
	rc = test_value_set();
	TEST_ZERO_CHK(rc);
	rc = test_argument_sanitize();
	TEST_ZERO_CHK(rc);
	rc = test_args_set();
	TEST_ZERO_CHK(rc);

	return 0;
}
TESTSUITE_REGISTER(libuc_args, test_libargs_uc);
