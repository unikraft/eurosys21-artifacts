#include <libfdt.h>
#include <ofw/fdt.h>
#include <testsuite.h>
#include <kvm/config.h>

#define DTB_PROPERTY_NAME_MAX 20
#define DTB_PROPERTY_COMPATIBLE_MAX 60
#define DTB_PROPERTY_NR_REPEAT 5

typedef struct test_u32_property  {
	/* Property to be searched */
	char name[DTB_PROPERTY_NAME_MAX];
	/* Result of each property search */
	int result[DTB_PROPERTY_NR_REPEAT];
	/* Offset into the property */
	int offset[DTB_PROPERTY_NR_REPEAT];
	/* Number of the time to repeat with corresponding offset */
	int chk_repeat;
} test_u32_property_t;

typedef struct test_cell_property {
	/* Property to be searched */
	char name[DTB_PROPERTY_NAME_MAX];
	/* Result of each property search */
	int result;
} test_cell_property_t;

typedef struct test_address_range {
	char name[DTB_PROPERTY_COMPATIBLE_MAX];
	int offset;
} test_address_range_t;

typedef struct test_address_compatiblity {
	char *name[DTB_PROPERTY_COMPATIBLE_MAX];
	int result;
} test_address_compatiblity_t;

typedef struct test_intr_property {
	char *compat[DTB_PROPERTY_COMPATIBLE_MAX];
	int result;
} test_intr_prop_t;

static test_u32_property_t prop_search_test_case[] = {
	{"interrupt-parent", {0}, {0}, 1},
	{"#address-cells", {0}, {0}, 1},
	{"interrupt-ell", {-FDT_ERR_NOTFOUND}, {0}, 1},
};

static test_cell_property_t cell_prop[] = {
	{"#interrupt-cells", 3},
};

static test_address_compatiblity_t test_compat[] = {
	{.name = {"simple-bus", NULL}, 0},
};

static test_intr_prop_t test_intr[] = {
	{.compat = {"arm,sbsa-gwdt", NULL}, 27},
};


static int test_fdt_get_property(void)
{
	uint32_t len;
	int rc = 0;
	int i;
	int cnt = 0;
	test_u32_property_t *prop = &prop_search_test_case[0];

	for (i = 0; i < (sizeof(prop_search_test_case) /
			sizeof(prop_search_test_case[0]));
	     i++, prop++) {
		rc = fdt_getprop_u32_by_offset(_libkvmplat_cfg.dtb,
				prop->offset[0], prop->name, &len);

		if (rc == prop->result[0]) {
			cnt++;
			printf("Success in reading the %s\n", prop->name);
		} else {
			printf("id: %d prop: %s Expected: %d, Returned: %d\n", i + 1 ,
				prop->name, prop->result[0], rc);
		}
	}

	return cnt;
}

static int test_interrupt_cell_property(void)
{
	int i, rc;
	test_cell_property_t *prop = &cell_prop[0];

	for (i = 0; i < sizeof(cell_prop) / sizeof(cell_prop[0]); i++, prop++) {
		rc = fdt_interrupt_cells(_libkvmplat_cfg.dtb, 0);
		if (rc == prop->result) {
			printf("Success in reading the %s\n", prop->name);
		} else {
			printf("id: %d, name: %s Expected: %d, Returned %d\n",
			       i+1, prop->name, prop->result, rc);
		}
	}
}

static int test_compatiblity_property(void)
{
	int i, rc;
	test_address_compatiblity_t *prop;

	prop = &test_compat[0];

	for (i = 0; i < sizeof(test_compat) / sizeof(test_compat[0]); i++, prop++) {
		printf("fetching the compatible %s\n", prop->name[0]);
		rc = fdt_node_offset_by_compatible_list(_libkvmplat_cfg.dtb, 0,
				prop->name);
		if (prop->result >= 0) {
			if(rc >= 0) {
				printf("Success in compatibility %s\n",
						prop->name[0]);
			} else {
				printf("Expecting %s but returned %d\n",
						prop->name[0], rc);
			}
		} else {
			if(rc >= 0) {
				printf("Expected failure in %s but got %d\n",
						prop->name[0], rc);
			} else  {
				printf("Expected failure in %s but got %d\n",
						prop->name[0], rc);
			}
		}
	}
	return 0;
}

static int test_intr_prop(void)
{
	int rc, i, offset, size, addr, j;
	test_intr_prop_t *prop;
	fdt32_t *test;

	prop =  &test_intr[0];

	for (i = 0; i < sizeof(test_intr) / sizeof(test_intr[0]); i++, prop++) {
		offset = fdt_node_offset_by_compatible_list(_libkvmplat_cfg.dtb,
				0, prop->compat);
		if (offset <= 0) {
			printf("Failed to find the compat %s\n",
					prop->compat[0]);
			continue;
		}

		rc = fdt_get_interrupt(_libkvmplat_cfg.dtb, offset, 0, &size, &test);
		if (rc == 0) {
			printf("Success interrupt was at %p with size %d\n", test, size);

			for (j = 0; j < size; j++)
				printf("interrupt %d \n", fdt32_to_cpu(*(test + j)));
		} else {
			printf("Failed(%d) interrupt was at %d with size %d\n", rc, addr, size);
		}
	}
	return 0;
}

int test_fdt(void)
{
	test_fdt_get_property();
	test_interrupt_cell_property();
	test_compatiblity_property();
	test_intr_prop();
}
TESTSUITE_REGISTER(fdt, test_fdt);
