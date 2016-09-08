/*
 * Copyright © 2016 Sören Tempel
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <semaphore.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "util.h"
#include "token.h"
#include "queue.h"
#include "scanner.h"
#include "turing.h"
#include "parser.h"

int
main(int argc, char **argv)
{
	parerr ret;
	dtm *tm;
	parser *par;
	FILE *fd;
	char *fc, *fp, *in;
	struct stat st;

	if (argc <= 2) {
		fprintf(stderr, "USAGE: %s FILE INPUT\n", argv[0]);
		return 1;
	}

	fp = argv[1];
	in = argv[2];

	if (stat(fp, &st))
		die("stat failed");
	else
		fc = emalloc(st.st_size + 1);

	if (!(fd = fopen(fp, "r")))
		die("fopen failed");
	if (fread(fc, sizeof(char), st.st_size, fd) != st.st_size)
		die("short read");

	if (fclose(fd))
		die("fclose failed");

	fc[st.st_size] = '\0';
	par = newparser(fc);

	tm = newtm();
	if ((ret = parsetm(par, tm)) != PAR_OK) {
		strparerr(par, ret, fp, stdout);
		return 1;
	}

	free(fc);
	freeparser(par);

	writetape(tm, in);
	if (runtm(tm))
		return 1;
	else
		return 0;
}
