/* SPDX-License-Identifier: LGPL-2.0-or-later */
/*
 *      Unikraft port of POSIX Threads Library for embedded systems
 *      Copyright(C) 2019 Costin Lupu, University Politehnica of Bucharest
 *
 *      This library is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public
 *      License as published by the Free Software Foundation; either
 *      version 2 of the License, or (at your option) any later version.
 *
 *      This library is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      Lesser General Public License for more details.
 *
 *      You should have received a copy of the GNU Lesser General Public
 *      License along with this library in the file COPYING.LIB;
 *      if not, write to the Free Software Foundation, Inc.,
 *      59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#ifdef _GNU_SOURCE
#include <uk/arch/limits.h>
#endif
#include <pthread.h>
#include <implement.h>


/* TODO We currently do not support guards */
int pthread_attr_setguardsize(pthread_attr_t *attr, size_t guardsize)
{
	if (guardsize != 0)
		return EINVAL;

	return 0;
}

int pthread_attr_getguardsize(const pthread_attr_t *attr, size_t *guardsize)
{
	if (!guardsize)
		return EINVAL;

	*guardsize = 0;

	return 0;
}

int pthread_attr_setstack(pthread_attr_t *attr,
		void *stackaddr, size_t stacksize)
{
	int rc;

	rc = pthread_attr_setstacksize(attr, stacksize);
	if (rc)
		goto out;

	rc = pthread_attr_setstackaddr(attr, stackaddr);

out:
	return rc;
}

int pthread_attr_getstack(const pthread_attr_t *attr,
		void **stackaddr, size_t *stacksize)
{
	int rc;

	rc = pthread_attr_getstacksize(attr, stacksize);
	if (rc)
		goto out;

	rc = pthread_attr_getstackaddr(attr, stackaddr);

out:
	return rc;
}

#ifdef _GNU_SOURCE
int pthread_getattr_np(pthread_t thread, pthread_attr_t *attr)
{
	pte_thread_t *tp = (pte_thread_t *) thread.p;
	struct uk_thread *_uk_thread;
	pthread_attr_t _attr;
	prio_t prio;
	int rc;

	if (tp == NULL || tp->threadId == NULL)
		return ENOENT;

	if (attr == NULL || *attr == NULL)
		return EINVAL;

	_uk_thread = tp->threadId;
	_attr = *attr;
	_attr->stackaddr = _uk_thread->stack;
	_attr->stacksize = __STACK_SIZE;

	_attr->detachstate = (_uk_thread->detached
		? PTHREAD_CREATE_DETACHED : PTHREAD_CREATE_JOINABLE);

	rc = uk_thread_get_prio(_uk_thread, &prio);
	if (rc == 0)
		_attr->param.sched_priority = prio;

	/* TODO inheritsched and contentionscope */

	return 0;
}

int pthread_setname_np(pthread_t thread, const char *name)
{
	pte_thread_t *tp = (pte_thread_t *) thread.p;
	struct uk_thread *_uk_thread;
	size_t len;

	if (tp == NULL || tp->threadId == NULL)
		return ENOENT;

	_uk_thread = tp->threadId;

	len = strnlen(name, 16);
	if (len > 15)
		return ERANGE;

	_uk_thread->name = name;

	return 0;
}

int pthread_getname_np(pthread_t thread, char *name, size_t len)
{
	pte_thread_t *tp = (pte_thread_t *) thread.p;
	struct uk_thread *_uk_thread;
	size_t _len;

	if (tp == NULL || tp->threadId == NULL)
		return ENOENT;

	_uk_thread = tp->threadId;

	_len = strlen(_uk_thread->name);
	if (len < _len + 1)
		return ERANGE;

	sprintf(name, _uk_thread->name);

	return 0;
}

#endif /* _GNU_SOURCE */
