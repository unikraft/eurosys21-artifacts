#include <uk/assert.h>
#include <uk/print.h>
#include <stdint.h>

#define  TEST_ZERO_CHK(val)		\
    do {				\
        UK_ASSERT(val == 0);		\
	uk_pr_info(#val " was zero\n"); \
    } while(0)

#define  TEST_NOT_ZERO_CHK(val)			\
    do {					\
        UK_ASSERT(val != 0);			\
	uk_pr_info(#val " was %d\n", val);	\
    } while(0)

#define  TEST_SNUM_EQUALS(a, b)				\
    do {						\
        UK_ASSERT((a) == (b));				\
	uk_pr_info("Expression " #a " was %ld and ", (long)(a));\
	uk_pr_info("Expression " #b " was %ld\n", (long)(b));	\
    } while(0)

#define  TEST_PTR_EQUALS(a, b)				\
    do {						\
        UK_ASSERT((a) == (b));				\
	uk_pr_info("Expression " #a " was %p and ",(void *)(uintptr_t)(a));\
	uk_pr_info("Expression " #b " was %p\n",(void *)(uintptr_t)(b));\
    } while(0)

#define  TEST_UNUM_EQUALS(a, b)				\
    do {						\
        UK_ASSERT((a) == (b));				\
	uk_pr_info("Expression " #a " was %lu and ",(unsigned long)(a));\
	uk_pr_info("Expression " #b " was %lu\n",(unsigned long)(b));	\
    } while(0)

#define  TEST_EXPR(expr)				\
    do {						\
	UK_ASSERT((expr));				\
	uk_pr_info(#expr " evaluated to true\n");	\
    } while(0)

#define TEST_NOT_NULL(val)				\
	do {						\
		UK_ASSERT(val);				\
		uk_pr_info(#val " reference %p\n", val);\
	} while(0)

#define TEST_NULL(val)					\
	do {						\
		UK_ASSERT(!val);			\
		uk_pr_info(#val " reference is null\n");\
	} while(0)

#define TEST_BYTES_EQUALS(ptr1, ptr2, size)				\
	do {								\
		UK_ASSERT(memcmp(ptr1, ptr2, size) == 0);		\
		uk_pr_info("Byte by byte comparison of pointers ");	\
		uk_pr_info(#ptr1 "@%p and ", ptr1);			\
		uk_pr_info(#ptr2 "@%p", ptr2);				\
		uk_pr_info(" of size %d were equal\n", size);		\
	} while(0)
