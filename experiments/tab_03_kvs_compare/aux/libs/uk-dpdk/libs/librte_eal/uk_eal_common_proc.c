#include <rte_eal.h>
#include <errno.h>
#include <time.h>
#include <rte_errno.h>
#include <uk/print.h>

int rte_mp_sendmsg(struct rte_mp_msg *msg)
{
	return -EINVAL;
}

int rte_mp_request_async(struct rte_mp_msg *req, const struct timespec *ts,
			 rte_mp_async_reply_t clb)
{
	return -EINVAL;
}

int rte_mp_reply(struct rte_mp_msg *msg, const char *peer)
{
	return -EINVAL;
}

int rte_mp_request_sync(struct rte_mp_msg *req, struct rte_mp_reply *reply,
			const struct timespec *ts)
{
	return -EINVAL;
}

int rte_mp_action_register(const char *name, rte_mp_t action)
{
	uk_pr_warn("%s action not supported\n", name);
	rte_errno = ENOTSUP;
	return -1;
}
