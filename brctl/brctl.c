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
#include <string.h>
#include <sys/errno.h>
#include "libbridge.h"
#include "config.h"

#include "brctl.h"

static void help()
{
	printf("commands:\n");
	command_helpall();
}

int main(int argc, char *argv[])
{
	const struct command *cmd;

	if (argc < 2)
		goto help;

	if (strcmp(argv[1], "-V") == 0) {
		printf("%s, %s\n", PACKAGE, VERSION);
		return 0;
	}

	if (br_init()) {
		fprintf(stderr, "can't setup bridge control: %s\n",
			strerror(errno));
		return 1;
	}

	if ((cmd = command_lookup(argv[1])) == NULL) {
		fprintf(stderr, "never heard of command [%s]\n", argv[1]);
		goto help;
	}
	
	if (argc < cmd->nargs + 2) {
		fprintf(stderr, "incorrect number of arguments for command\n");
		goto help;
	}

	return cmd->func(++argv);

help:
	help();
	return 1;
}
