/*
 * Copyright (C) 2000 Lennert Buytenhek
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include "libbridge.h"
#include "libbridge_private.h"

#define MAX_BRIDGES	1024	/* arbitrary */
#define MAX_PORTS	4096

int br_socket_fd;
struct bridge *bridge_list;

static void br_nuke_bridge(struct bridge *b)
{
	struct port *p, *n;

	for (p = b->firstport; p; p = n) {
		n = p->next;
		free(p);
	}

	free(b);
}

static int br_make_port_list(struct bridge *br)
{
	int err;
	int i;
	int ifindices[MAX_PORTS];
	struct port *p, **top;

	memset(ifindices, 0, sizeof(ifindices));
	if (br_device_ioctl(br, BRCTL_GET_PORT_LIST, (unsigned long)ifindices,
			    MAX_PORTS, 0) < 0)
		return errno;

	top = &br->firstport;
	for (i = 0; i < MAX_PORTS; i++) {
		if (!ifindices[i])
			continue;

		p = malloc(sizeof(struct port));
		if (!p) {
			err = -ENOMEM;
			goto error_out;
		}

		p->next = NULL;
		p->ifindex = ifindices[i];
		p->parent = br;
		p->index = i;
		*top = p;
		top = &p->next;
	}

	return 0;

 error_out:
	p = br->firstport;
	while (p) {
		struct port *n = p->next;
		free(p);
		p = n;
	}
	br->firstport = NULL;

	return err;
}

static int br_make_bridge_list(void)
{
	int i, num;
	int ifindices[MAX_BRIDGES];

	num = br_get_br(BRCTL_GET_BRIDGES, (unsigned long)ifindices, 
			MAX_BRIDGES);
	if (num < 0) {
		fprintf(stderr, "Get bridge indices failed: %s\n",
			strerror(errno));
		return errno;
	}
	bridge_list = NULL;
	for (i = 0;i < num; i++) {
		struct bridge *br;

		br = malloc(sizeof(struct bridge));
		if (!br)
			return -ENOMEM;

		memset(br, 0, sizeof(struct bridge));
		br->ifindex = ifindices[i];
		br->firstport = NULL;
		if ( br_make_port_list(br)) {
			/* ignore the problem could just be a race! */
			free(br);
			continue;
		}

		br->next = bridge_list;
		bridge_list = br;
	}

	return 0;
}

int br_init()
{
	int err;

	if ((br_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return errno;

	if (br_get_version() != BRCTL_VERSION) {
		fprintf(stderr, "bridge utilities not compatiable with kernel version\n");
		exit(1);
	}

	if ((err = br_make_bridge_list()) != 0)
		return err;

	return 0;
}

int br_refresh()
{
	struct bridge *b;

	b = bridge_list;
	while (b != NULL) {
		struct bridge *bnext;

		bnext = b->next;
		br_nuke_bridge(b);
		b = bnext;
	}

	return br_make_bridge_list();
}
