#include <rte_alarm.h>
#include <errno.h>
/**
 * TODO:
 * Timer support for the unikraft
 */
int rte_eal_alarm_init(void)
{
	return -EINVAL;
}

/**
 * TODO:
 * Setting up the callback for the handler
 */
int _alarm_callback(void *arg)
{
	return -EINVAL;
}

/**
 * TODO:
 * Setting the alarm
 */
int
rte_eal_alarm_set(uint64_t us, rte_eal_alarm_callback cb_fn, void *cb_arg)
{
	return -EINVAL;
}

/**
 * TODO:
 * Cancel the alarm
 */
int
rte_eal_alarm_cancel(rte_eal_alarm_callback cb_fn, void *cb_arg)
{
	return -EINVAL;
}
