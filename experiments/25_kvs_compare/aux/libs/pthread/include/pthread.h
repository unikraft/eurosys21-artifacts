/* SPDX-License-Identifier: LGPL-2.0-or-later */
/*
 *      Unikraft port of POSIX Threads Library for embedded systems
 *      Copyright(C) 2019, University Politehnica of Bucharest
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

#ifndef __GLUE_PTHREAD_H__
#define __GLUE_PTHREAD_H__

#define _GNU_SOURCE

#ifdef __cplusplus
extern "C" {
#endif

/* The C code in pthread.h is not guarded for C++ */
#include_next <pthread.h>

int pthread_atfork(void (*prepare)(void),
	void (*parent)(void), void (*child)(void));

/* C functions not implemented in pthread-embedded */
int pthread_attr_setguardsize(pthread_attr_t *attr, size_t guardsize);
int pthread_attr_getguardsize(const pthread_attr_t *attr, size_t *guardsize);

int pthread_attr_setstack(pthread_attr_t *attr,
		void *stackaddr, size_t stacksize);
int pthread_attr_getstack(const pthread_attr_t *attr,
		void **stackaddr, size_t *stacksize);


#ifdef _GNU_SOURCE
int pthread_setaffinity_np(pthread_t thread, size_t cpusetsize,
			   const cpu_set_t *cpuset);
int pthread_getaffinity_np(pthread_t thread, size_t cpusetsize,
			   cpu_set_t *cpuset);
int pthread_getattr_np(pthread_t thread, pthread_attr_t *attr);

int pthread_setname_np(pthread_t thread, const char *name);
int pthread_getname_np(pthread_t thread, char *name, size_t len);
#endif /* _GNU_SOURCE */

#ifdef __cplusplus
}
#endif

#endif /* __GLUE_PTHREAD_H__ */
