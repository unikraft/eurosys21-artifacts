#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <uk/essentials.h>

/* Import user configuration: */
#include <uk/config.h>

int main(int argc, char *argv[])
{
	int fd;
	const char *filename = "randomfile";
	unsigned long block_size;
	ssize_t rx;
	char *buf;
	unsigned long loop_cnt = 0;
	struct timespec start, end;
	unsigned long duration;
	filename = argv[1];
	
	block_size = atoi(argv[2]) * 1024;

	printf("Test with file %s, block size %lu\n", filename, block_size);

	buf = malloc(block_size);
	if (!buf) {
		printf("malloc failed\n");
		return 1;
	}

	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		printf("open failed\n");
		return 1;
	}

	clock_gettime(CLOCK_REALTIME, &start);

	while (1) {
		rx = read(fd, buf, block_size);
		if (rx < 0) {
			printf("read failed\n");
			return 1;
		} else if (rx == 0) {
			break;
		}
		loop_cnt++;
	}

	clock_gettime(CLOCK_REALTIME, &end);

	unsigned long delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
	printf("Total execution time: %lu usec\n", delta_us);

	duration = (end.tv_sec * 1000000000UL + end.tv_nsec) - (start.tv_sec * 1000000000UL + start.tv_nsec);
	printf("avg operation latency: %lu nsec\n", (duration / loop_cnt));

	close(fd);
	free(buf);
}
