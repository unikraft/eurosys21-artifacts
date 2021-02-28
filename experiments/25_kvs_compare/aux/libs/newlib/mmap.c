#include <sys/mman.h>
#include <uk/assert.h>
#include <uk/list.h>
#include <uk/alloc.h>
#include <errno.h>
#include <uk/errptr.h>
#include <uk/arch/types.h>
#include <uk/refcount.h>

struct mmap_alloc_list {
	void *addr;
	int length;
	struct uk_list_head next;
	__atomic refcnt;
};

static struct uk_list_head mmap_alloc_list = UK_LIST_HEAD_INIT(mmap_alloc_list);

static struct mmap_alloc_list *search_mmap_list(void *addr, int length)
{
	struct mmap_alloc_list *ref;

	uk_list_for_each_entry(ref, &mmap_alloc_list, next) {
		if (ref->addr <= addr &&
		   (ref->addr + ref->length) >= (addr + length)) {
			return ref;
		}
	}

	return NULL;
}

void *mmap(void *addr, size_t length, int prot __unused, int flags __unused,
	   int fd __unused, off_t offset __unused)
{
	int rc;
	struct mmap_alloc_list *ref;

	if (addr) {
		ref = search_mmap_list(addr, length);
		if (!ref) {
			uk_pr_err("Failed to map the address\n");
			rc = -EINVAL;
			goto err_set;
		}
		uk_refcount_acquire(&ref->refcnt);
		return ref->addr;
	} else {
		length = ALIGN_UP(length, (1 << __PAGE_SHIFT));
		addr = uk_palloc(uk_alloc_get_default(),
				 length >> __PAGE_SHIFT);

		if (!addr) {
			rc = -ENOMEM;
			uk_pr_err("Failed to allocate memory\n");
			goto err_set;
		}
		ref = uk_malloc(uk_alloc_get_default(),
				 sizeof(*ref));
		if (!ref) {
			rc = -ENOMEM;
			goto free_alloc;
		}
		ref->addr = addr;
		ref->length = length;
		uk_refcount_init(&ref->refcnt, 1);
		uk_list_add_tail(&ref->next, &mmap_alloc_list);
	}

free_alloc:
	uk_pfree(uk_alloc_get_default(), addr, (length >> __PAGE_SHIFT));
err_set:
	errno = rc;
	return ERR2PTR(-1);
}

int munmap(void *addr, size_t length)
{
	struct mmap_alloc_list *ref;
	int rc;

	if (addr) {
		ref = search_mmap_list(addr, length);
		if (!ref) {
			uk_pr_err("Failed to unmap the address\n");
			rc = -EINVAL;
			return -1;
		}
		rc = uk_refcount_release(&ref->refcnt);
		if (rc == 1)
			uk_list_del(&ref->next);
		return 0;
	}

	errno = -EINVAL;
	return -1;
}

/**
 *  Stub implementation
 */
int mlock(const void *addr, size_t len)
{
	return 0;
}
