#include <rte_interrupts.h>
#include <errno.h>


int rte_intr_rx_ctl(struct rte_intr_handle *intr_handle, int epfd,
		    int op, unsigned int vec, void *data)
{
	return -EINVAL;
}

int rte_intr_callback_unregister(const struct rte_intr_handle *intr_handle,
				 rte_intr_callback_fn cb, void *cb_arg)
{
	return -EINVAL;
}

int rte_intr_enable(const struct rte_intr_handle *intr_handle)
{
	return -EINVAL;
}

int rte_intr_disable(const struct rte_intr_handle *intr_handle)
{
	return -EINVAL;
}

int rte_intr_callback_unregister_pending(const struct rte_intr_handle *intr_handle,
					 rte_intr_callback_fn cb_fn, void *cb_arg,
					 rte_intr_unregister_callback_fn ucb_fn)
{
	return -EINVAL;
}
