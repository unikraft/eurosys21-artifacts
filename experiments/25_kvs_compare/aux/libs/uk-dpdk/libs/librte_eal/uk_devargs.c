#include <uk/essentials.h>
#include <rte_devargs.h>


int rte_eal_parse_devargs_str(const char *devargs_str __unused,
			      char **drvname __unused,
			      char **drvargs __unused)
{
	return 0;
}

int rte_devargs_parse(struct rte_devargs *da __unused,
		      const char *dev __unused)
{
	return 0;
}

int rte_devargs_insert(struct rte_devargs **da __unused)
{
	return 0;
}

int rte_devargs_add(enum rte_devtype devtype __unused,
		    const char *devargs_str __unused)
{
	return 0;
}

int rte_eal_devargs_add(enum rte_devtype devtype __unused, const char *devargs_str __unused)
{
	return 0;
}

int rte_devargs_remove(struct rte_devargs *devargs __unused)
{
	return 0;
}

unsigned int rte_devargs_type_count(enum rte_devtype devtype __unused)
{
	return 0;
}

unsigned int rte_eal_devargs_type_count(enum rte_devtype devtype __unused)
{
	return 0;
}

void rte_devargs_dump(FILE *f __unused)
{
	return ;
}

void rte_eal_devargs_dump(FILE *f __unused)
{
	return ;
}

struct rte_devargs *rte_devargs_next(const char *busname __unused,
				     const struct rte_devargs *start __unused)
{
	return NULL;
}
