/*
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License as
 *	published by the Free Software Foundation; either version 2 of
 *	the License, or (at your option) any later version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <netdb.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MAXREMOTE 128

struct sockaddr_in6 localaddr;
int numremote;
struct sockaddr_in6 remoteaddr[MAXREMOTE];
int sock;
int tap;
char *tapdevice;



int getaddr(const char *hostname, struct sockaddr_in6 *in)
{
	struct hostent *he;

	in->sin6_port = 0;

	if ((he = gethostbyname2(hostname, AF_INET6)) != NULL) {
		in->sin6_family = he->h_addrtype;
		memcpy(in->sin6_addr.s6_addr, he->h_addr_list[0], he->h_length);
		return 0;
	}

	if ((he = gethostbyname2(hostname, AF_INET)) != NULL) {
		in->sin6_family = AF_INET6;
		memset(in->sin6_addr.s6_addr, 0, 10);
		in->sin6_addr.s6_addr[10] = 0;
		in->sin6_addr.s6_addr[11] = 0;
		memcpy(in->sin6_addr.s6_addr + 12, he->h_addr_list[0], he->h_length);
		return 0;
	}

	return 1;
}

void handle()
{
	struct pollfd pfd[2];

	pfd[0].fd = tap;
	pfd[0].events = POLLIN;
	pfd[1].fd = sock;
	pfd[1].events = POLLIN;

	while (1) {
		unsigned char buf[2048];
		int len;

		if (poll(pfd, 2, -1) <= 0) {
			perror("poll");
			break;
		}

		if (pfd[0].revents & POLLIN) {
			int i;

			if ((len = read(pfd[0].fd, buf, 2048)) < 0) {
				perror("read");
				break;
			}
			for (i=0;i<numremote;i++)
				if (sendto(pfd[1].fd, buf+2, len-2, 0,
					   &remoteaddr[i],
					   sizeof(remoteaddr[i])) < 0) {
					perror("sendto");
					break;
				}
		}

		if (pfd[1].revents & POLLIN) {
			if ((len = read(pfd[1].fd, buf+2, 2046)) < 0) {
				perror("read");
				break;
			}
			if (write(pfd[0].fd, buf, len+2) < 0) {
				perror("write");
				break;
			}
		}
	}

	shutdown(sock, 2);
}

int main(int argc, char *argv[])
{
	int i;

	if (argc < 4) {
		fprintf(stderr, "syntax: %s <tapdevice> <localaddr or ::> <remotename>\n", argv[0]);
		return 1;
	}

	tapdevice = argv[1];

	if (getaddr(argv[2], &localaddr)) {
		fprintf (stderr, "Can't resolve local hostname/address %s\n",
			argv[2]);
		return 1;
	}

	numremote = argc - 3;
	for (i=3;i<argc;i++)
		if (getaddr(argv[i], &remoteaddr[i-3])) {
			fprintf(stderr,
				"can't resolve hostname %s\n",
				argv[i]);
			return 1;
		}

	if ((tap = open(tapdevice, O_RDWR)) < 0) {
		perror("open");
		return 1;
	}

	if ((sock = socket(AF_INET6, SOCK_RAW, 97)) < 0) {
		perror("socket");
		return 1;
	}

	if (bind(sock, &localaddr, sizeof(localaddr)) < 0) {
		perror("bind");
		return 1;
	}

	handle();

	return 0;
}
