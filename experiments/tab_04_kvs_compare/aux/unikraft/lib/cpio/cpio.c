/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Authors: Robert Hrusecky <roberth@cs.utexas.edu>
 *          Omar Jamil <omarj2898@gmail.com>
 *          Sachin Beldona <sachinbeldona@utexas.edu>
 *
 * Copyright (c) 2017, NEC Europe Ltd., NEC Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * THIS HEADER MAY NOT BE EXTRACTED OR MODIFIED IN ANY WAY.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <uk/cpio.h>
#include <uk/essentials.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define CPIO_MAGIC_NEWC "070701"
#define CPIO_MAGIC_CRC "070702"
#define FILE_TYPE_MASK 0170000
#define DIRECTORY_BITS 040000
#define FILE_BITS 0100000

#define ALIGN_4(ptr) ((void *)ALIGN_UP((uintptr_t)(ptr), 4))

#define IS_FILE_OF_TYPE(mode, bits) (((mode) & (FILE_TYPE_MASK)) == (bits))
#define IS_FILE(mode) IS_FILE_OF_TYPE((mode), (FILE_BITS))
#define IS_DIR(mode) IS_FILE_OF_TYPE((mode), (DIRECTORY_BITS))

#define GET_MODE(hdr) ((mode_t)strhex8_to_u32((hdr)->mode))

struct cpio_header {
	char magic[6];
	char inode_num[8];
	char mode[8];
	char uid[8];
	char gid[8];
	char nlink[8];
	char mtime[8];
	char filesize[8];
	char major[8];
	char minor[8];
	char ref_major[8];
	char ref_minor[8];
	char namesize[8];
	char chksum[8];
};

static bool valid_magic(struct cpio_header *header)
{
	return memcmp(header->magic, CPIO_MAGIC_NEWC, 6) == 0
	       || memcmp(header->magic, CPIO_MAGIC_CRC, 6) == 0;
}

/* Function to convert len digits of hexadecimal string loc
 * to an integer.
 * Returns the converted unsigned integer value on success.
 * Returns 0 on error.
 */
static unsigned int snhex_to_int(size_t len, char *loc)
{
	int val = 0;
	size_t i;

	for (i = 0; i < len; i++) {
		val *= 16;
		if (*(loc + i) >= '0' && *(loc + i) <= '9')
			val += (*(loc + i) - '0');
		else if (*(loc + i) >= 'A' && *(loc + i) <= 'F')
			val += (*(loc + i) - 'A') + 10;
		else if (*(loc + i) >= 'a' && *(loc + i) <= 'f')
			val += (*(loc + i) - 'a') + 10;
		else
			return 0;
	}
	return val;
}

static uint32_t strhex8_to_u32(char *loc)
{
	return snhex_to_int(8, loc);
}

static inline char *filename(struct cpio_header *header)
{
	return (char *)header + sizeof(struct cpio_header);
}

static char *absolute_path(char *path_to_prepend, char *path)
{
	int dir_slash_included =
	    *(path_to_prepend + strlen(path_to_prepend) - 1) == '/' ? 1 : 2;
	char *abs_path = (char *)malloc(strlen(path) + strlen(path_to_prepend)
					+ dir_slash_included);
	if (abs_path == NULL)
		return NULL;
	memcpy(abs_path, path_to_prepend, strlen(path_to_prepend));
	if (dir_slash_included == 2)
		*(abs_path + strlen(path_to_prepend)) = '/';
	memcpy(abs_path + strlen(path_to_prepend) + dir_slash_included - 1,
	       path, strlen(path));
	*(abs_path + strlen(path) + strlen(path_to_prepend) + dir_slash_included
	  - 1) = '\0';
	return abs_path;
}

static enum cpio_error read_section(struct cpio_header **header_ptr,
				    char *mount_loc, uintptr_t last)
{
	if (strcmp(filename(*header_ptr), "TRAILER!!!") == 0) {
		*header_ptr = NULL;
		return CPIO_SUCCESS;
	}

	if (!valid_magic(*header_ptr)) {
		*header_ptr = NULL;
		return -CPIO_INVALID_HEADER;
	}

	if (mount_loc == NULL) {
		*header_ptr = NULL;
		return -CPIO_NO_MOUNT_LOCATION;
	}

	struct cpio_header *header = *header_ptr;
	char *path_from_root = absolute_path(mount_loc, filename(header));

	if (path_from_root == NULL) {
		*header_ptr = NULL;
		return -CPIO_NOMEM;
	}
	mode_t header_mode = GET_MODE(header);
	uint32_t header_filesize = strhex8_to_u32(header->filesize);
	uint32_t header_namesize = strhex8_to_u32(header->namesize);

	if ((uintptr_t)header + sizeof(struct cpio_header) > last) {
		*header_ptr = NULL;
		return -CPIO_MALFORMED_FILE;
	}
	if (IS_FILE(header_mode) && header_filesize != 0) {
		uk_pr_debug("Creating file %s...\n", path_from_root);
		int fd = open(path_from_root, O_CREAT | O_RDWR);

		if (fd < 0) {
			*header_ptr = NULL;
			return -CPIO_FILE_CREATE_FAILED;
		}
		uk_pr_debug("File %s created\n", path_from_root);
		char *data_location = (char *)ALIGN_4(
		    (char *)(header) + sizeof(struct cpio_header)
		    + header_namesize);

		if ((uintptr_t)data_location + header_filesize > last) {
			*header_ptr = NULL;
			return -CPIO_MALFORMED_FILE;
		}
		uint32_t bytes_to_write = header_filesize;
		int bytes_written = 0;

		while (bytes_to_write > 0) {
			if ((bytes_written =
				 write(fd, data_location + bytes_written,
				       bytes_to_write))
			    < 0) {
				*header_ptr = NULL;
				return -CPIO_FILE_WRITE_FAILED;
			}
			bytes_to_write -= bytes_written;
		}
		if (chmod(path_from_root, header_mode & 0777) < 0)
			uk_pr_info("chmod on file %s failed\n", path_from_root);
		if (close(fd) < 0) {
			*header_ptr = NULL;
			return -CPIO_FILE_CLOSE_FAILED;
		}
	} else if (IS_DIR(header_mode)) {
		if (strcmp(".", filename(header)) != 0
		    && mkdir(path_from_root, header_mode & 0777) < 0) {
			*header_ptr = NULL;
			return -CPIO_MKDIR_FAILED;
		}
	}
	free(path_from_root);
	struct cpio_header *next_header = (struct cpio_header *)ALIGN_4(
	    (char *)header + sizeof(struct cpio_header) + header_namesize);

	next_header = (struct cpio_header *)ALIGN_4((char *)next_header
						    + header_filesize);
	*header_ptr = next_header;
	return CPIO_SUCCESS;
}

enum cpio_error cpio_extract(char *mount_loc, void *memory_region, size_t len)
{
	enum cpio_error error = CPIO_SUCCESS;
	struct cpio_header *header = (struct cpio_header *)(memory_region);
	struct cpio_header **header_ptr = &header;
	uintptr_t end = (uintptr_t)header;

	if (mount_loc == NULL)
		return -CPIO_NO_MOUNT_LOCATION;

	while (error == CPIO_SUCCESS && header != NULL) {
		error = read_section(header_ptr, mount_loc, end + len);
		header = *header_ptr;
	}
	return error;
}
