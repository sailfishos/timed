#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <sys/timerfd.h>

#ifndef TFD_TIMER_CANCELON_SET
#define TFD_TIMER_CANCELON_SET (1 << 1)
#endif

#ifndef O_CLOEXEC
#define O_CLOEXEC   02000000        /* set close_on_exec */
#endif
#ifndef TFD_CLOEXEC
#define TFD_CLOEXEC O_CLOEXEC
#endif
#ifndef TFD_NONBLOCK
#define TFD_NONBLOCK O_NONBLOCK
#endif

int main(int , char *[])
{
	struct itimerspec its;
	struct pollfd pollfd[2];

	pollfd[0].fd = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK|TFD_CLOEXEC);
	if (pollfd[0].fd < 0) {
		printf("create error 1: %m\n");
		_exit(2);
	}
	pollfd[0].events = POLLIN;

	pollfd[1].fd = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK|TFD_CLOEXEC);
	if (pollfd[1].fd < 0) {
		printf("create error 2: %m\n");
		_exit(2);
	}
	pollfd[1].events = POLLIN;

	for (;;) {
		printf("\n");

		memset(&its, 0, sizeof(struct itimerspec));
		clock_gettime(CLOCK_REALTIME, &its.it_value);
		its.it_value.tv_sec += 10;

		if (timerfd_settime(pollfd[0].fd, TFD_TIMER_ABSTIME, &its, NULL) < 0) {
			printf("settime error 1: %m\n");
			_exit(3);
		} else {
			printf("settime +10s 1\n");
		}

		if (timerfd_settime(pollfd[1].fd, TFD_TIMER_ABSTIME|TFD_TIMER_CANCELON_SET, &its, NULL) < 0) {
			printf("settime error: 2: %m\n");
			_exit(3);
		} else {
			printf("settime +10s 2\n");
		}

		printf("poll\n");
		if (poll(pollfd, 2, -1) < 0) {
			if (errno == EAGAIN || errno == EINTR)
				continue;
			_exit(4);
		}

		if (pollfd[0].revents) {
			uint64_t count;

			if (read(pollfd[0].fd, &count, sizeof(uint64_t)) < 0)
				printf("read error 1: %m\n");
			else
				printf("read 1: %u\n", (unsigned int)count);

			printf("wakeup 1\n");
		}

		if (pollfd[1].revents) {
			uint64_t count;

			if (read(pollfd[1].fd, &count, sizeof(uint64_t)) < 0)
				printf("read error 2: %m\n");
			else
				printf("read 2: %u\n", (unsigned int)count);

			printf("wakeup 2\n");
		}
	}

	return EXIT_SUCCESS;
}
