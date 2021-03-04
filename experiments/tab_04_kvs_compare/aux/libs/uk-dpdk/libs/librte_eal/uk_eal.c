#include <eal_internal_cfg.h>
#include <eal_private.h>
#include <pthread.h>
#include <getopt.h>
#include <eal_options.h>
#include <generic/rte_spinlock.h>
#include <rte_spinlock.h>
#include <rte_lcore.h>
#include <rte_eal_memconfig.h>
#include <eal_hugepages.h>
#include <eal_filesystem.h>
#include <uk/print.h>
#include <rte_memory.h>
#include <rte_memzone.h>
#include <rte_errno.h>
#include "eal_memcfg.h"
#include <eal_thread.h>

#define DEFAULT_RUNTIME_PATH "/tmp/"
#define MEMSIZE_IF_NO_HUGE_PAGE RTE_UK_MEMSIZE

/* internal configuration */
struct internal_config internal_config;
struct lcore_config lcore_config[RTE_MAX_LCORE];
static struct rte_mem_config early_mem_config;
/* Address of global and public configuration */
static struct rte_config rte_config = {
		.mem_config = &early_mem_config,
};

/**
 * Add a weak alias to the rte_eal_cpu_init
 */

static void eal_usage(const char *prgname)
{
	printf("\nUsage: %s ", prgname);
	eal_common_usage();
	/**
	 * TODO:
	 * Add user option for unikraft.
	 */
}

enum rte_proc_type_t rte_eal_process_type(void)
{
	return rte_config.process_type;
}

struct rte_config *rte_eal_get_configuration(void)
{
	return &rte_config;
}

/**
 * Support only for the primary process.
 */
enum rte_proc_type_t eal_proc_type_detect(void)
{
	return RTE_PROC_PRIMARY;
}

const char *rte_eal_get_runtime_dir(void)
{
	return DEFAULT_RUNTIME_PATH;
}

static int eal_parse_args(int argc, char **argv)
{
	int rc;
	char **argvopt;
	int option_index = 0;
	char *prgname = argv[0];
	const int old_optind = optind;
	const int old_optopt = optopt;
	char * const old_optarg = optarg;
	int opt;

	argvopt = argv;
	optind = 1;
#if !defined(CONFIG_HUGEPAGE_DIR) && !defined(CONFIG_HUGEPAGE_FILE)
	internal_config.no_hugetlbfs = 1;
#endif /* !CONFIG_HUGEPAGE_DIR && !CONFIG_HUGEPAGE_FILE */
	while ((opt = getopt_long(argc, argvopt, eal_short_options,
				  eal_long_options, &option_index)) >= 0) {
		/* getopt is not happy, stop right now */
		if (opt == '?') {
			uk_pr_err("Get opt long is not happy\n");
			eal_usage(prgname);
			rc = -1;
			goto out;
		}

		printf("Opt: %d and optarg: %s\n", opt, optarg);

		switch (opt) {
		case 'h':
			eal_usage(prgname);
			exit(EXIT_SUCCESS);
		case 'd':
			RTE_LOG(ERR, EAL, "Unikraft does not support loading modules\n");
			break;
		case OPT_PROC_TYPE_NUM:
			internal_config.process_type = RTE_PROC_PRIMARY;
			break;
#ifdef CONFIG_HUGEPAGE_DIR
		case OPT_HUGE_DIR_NUM:
			internal_config.hugepage_dir = strdup(optarg);
			break;
#endif /* CONFIG_HUGEPAGE_DIR */
#ifdef CONFIG_HUGEPAGE_FILE
		case OPT_FILE_PREFIX_NUM:
			internal_config.hugefile_prefix = strdup(optarg);
			break;
#endif /* CONFIG_HUGEPAGE_FILE */
#ifdef CONFIG_SOCKET_MEM_NUM
		case OPT_SOCKET_MEM_NUM:
			if (eal_parse_socket_mem(optarg) < 0) {
				RTE_LOG(ERR, EAL, "invalid parameters for --"
						OPT_SOCKET_MEM "\n");
				eal_usage(prgname);
				rc = -1;
				goto out;
			}
			break;
#endif /* CONFIG_SOCKET_MEM_NUM */
#ifdef CONFIG_VIRTADDR_NUM
		case OPT_BASE_VIRTADDR_NUM:
			if (eal_parse_base_virtaddr(optarg) < 0) {
				RTE_LOG(ERR, EAL, "invalid parameter for --"
						OPT_BASE_VIRTADDR "\n");
				eal_usage(prgname);
				rc = -1;
				goto out;
			}
			break;
#endif /* CONFIG_VIRTADDR_NUM */
#ifdef CONFIG_VFIO_INTR_NUM
		case OPT_VFIO_INTR_NUM:
			if (eal_parse_vfio_intr(optarg) < 0) {
				RTE_LOG(ERR, EAL, "invalid parameters for --"
						OPT_VFIO_INTR "\n");
				eal_usage(prgname);
				rc = -1;
				goto out;
			}
			break;
#endif /* CONFIG_VFIO_INTR_NUM */

#ifdef CONFIG_UIO_DEV
		case OPT_CREATE_UIO_DEV_NUM:
			internal_config.create_uio_dev = 1;
			break;
#endif /* CONFIG_UIO_DEV_NUM */

#ifdef CONFIG_MBUF_POOL_OPS_NAME
		case OPT_MBUF_POOL_OPS_NAME_NUM:
			internal_config.mbuf_pool_ops_name = optarg;
			break;
#endif /* CONFIG_MBUF_POOL_OPS_NAME */

		default:
			/**
			 * Parse common options later to enable overriding
			 * default common options
			 */
			rc = eal_parse_common_option(opt, optarg,
						     &internal_config);

			/* common parser handled this option */
			if (rc == 0)
				continue;
			/* common parser is not happy */
			if (rc < 0) {
				eal_usage(prgname);
				rc = -1;
				goto out;
			} else if (opt < OPT_LONG_MIN_NUM && isprint(opt))
				uk_pr_err("Option %c is not supported on Unikraft\n",
					opt);
			else if (opt >= OPT_LONG_MIN_NUM &&
				 opt < OPT_LONG_MAX_NUM)
				 uk_pr_err("Option %s is not supported on Unikraft\n",
					eal_long_options[option_index].name);
			else
				uk_pr_err( "Option %d is not supported on Unikraft\n",
					opt);

			eal_usage(prgname);
			rc = -1;
			goto out;
		}
	}
	if (eal_adjust_config(&internal_config) != 0) {
		rc = -1;
		goto out;
        }

	/* sanity checks */
	if (eal_check_common_options(&internal_config) != 0) {
	        eal_usage(prgname);
	        rc = -1;
	        goto out;
	}

	if (optind >= 0)
		argv[optind-1] = prgname;
	rc = optind-1;

out:
	/* restore getopt lib */
	optind = old_optind;
	optopt = old_optopt;
	optarg = old_optarg;

	return rc;
}

/**
 * TODO:
 * Need a way to identify the rank and the channel from the memory DIMM.
 * This information should be across the different platforms. The
 * arch part of the code should also be considered
 */
static int rte_eal_memory_dev_init(void)
{
	return 0;
}

/**
 * TODO:
 * rte_physmem_desc: The physical page description. From a guest perspective
 * we have a 1:1 pa to va mapping. As unikraft does not yet support huge page
 * implementation.
 */
int rte_eal_get_physmem_layout(void)
{
	return 0;
}

const char * rte_eal_mbuf_user_pool_ops(void)
{
	return "ring_uk_netbuf";
}

static void rte_eal_config_create(void)
{
	void *rte_config_mem_addr;

	rte_config_mem_addr = malloc(sizeof(*rte_config.mem_config));
	if(!rte_config_mem_addr) {
		uk_pr_err("Failed to allocate memory for memory config\n");
		return ;
	}

	memcpy(rte_config_mem_addr, &early_mem_config, sizeof(early_mem_config));
	rte_config.mem_config->mem_cfg_addr = (uintptr_t) rte_config_mem_addr;
	rte_config.mem_config->dma_maskbits = 0;
	rte_config.process_type = RTE_PROC_PRIMARY;

	uk_pr_info("config create success\n");
}

static void rte_config_init(void)
{
	rte_config.process_type = internal_config.process_type;

	switch (rte_config.process_type){
	case RTE_PROC_PRIMARY:
		uk_pr_debug("Initializing the primary process config\n");
		rte_eal_config_create();
		break;
	default:
		uk_pr_err("Process type(%d) not supported\n",
			  rte_config.process_type);
	}
}

int rte_eal_has_hugepages(void)
{
	return 0;
}

enum rte_iova_mode rte_eal_iova_mode(void)
{
	return rte_eal_get_configuration()->iova_mode;
}

/**
 * Initialization code for Unikraft
 *
 * ) Argument parser.
 * ) Device arg parsed
 * ) Config initialization
 * TODO:
 * ) CPU Initialization.
 * ) Memory zone Initialization
 * ) Memory Initialization.
 */
int rte_eal_init(int argc, char **argv)
{
	int rc;

	eal_reset_internal_config(&internal_config);

	rc = rte_eal_cpu_init();
	if (rc < 0) {
		uk_pr_err("Failed to initialize the CPU\n");
		return rc;
	}

	uk_pr_debug("initialized the cpu_init\n");
	internal_config.no_shconf = 1;
	internal_config.memory = MEMSIZE_IF_NO_HUGE_PAGE;
	internal_config.legacy_mem = 1;

	/**
	 * Parse the argument.
	 */
	rc = eal_parse_args(argc, argv);
	if (rc < 0) {
		uk_pr_err("Failed to parse the argument to library\n");
		return rc;
	}

	uk_pr_debug("parsed argument\n");
	
	/**
	 * Process the arguments on the device
	 */
	rc = eal_option_device_parse();
	if (rc < 0) {
		uk_pr_err("Failed to parse the device\n");
		return rc;
	}
	uk_pr_debug("dev_args parsed\n");

	rte_config_init();

	/**
	 * Configure the heap based on the huge page information. 
	 */
	rc = eal_hugepage_info_init();
	if (rc < 0) {
		uk_pr_err("Failed to fetch hugetable info\n");
		return rc;
	}

	/**
	 * Memzone initialization configure the fbarray.
	 */
	rc = rte_eal_memzone_init();
	if (rc < 0) {
		uk_pr_err("Failed to initialize the memory zone\n");
		return rc;
	}

	/**
	 * TODO:
	 * Check if we need changes to configure
	 * - memseg
	 * - memalloc
	 */
	rc = rte_eal_memory_init();
	if (rc < 0) {
		uk_pr_err("Failed to initialize the memory\n");
		return rc;
	}

	rc = rte_eal_malloc_heap_init();
	if (rc < 0) {
		uk_pr_err("Failed to initialize heap\n");
		return rc;
	}

	if (rte_eal_tailqs_init() < 0) {
		uk_pr_err("Cannot init tail queues for objects\n");
		rte_errno = EFAULT;
		return -1;
	}

	eal_thread_init_master(rte_config.master_lcore);

	rc = eal_uknetdev_init();
	if (rc < 0) {
		uk_pr_err("Failed(%d) to initializes the netdevice\n", rc);
		return rc;
	}
	return 0;
}
