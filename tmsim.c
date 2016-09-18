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
#include <semaphore.h>
#include <unistd.h>

#include "util.h"
#include "token.h"
#include "queue.h"
#include "scanner.h"
#include "turing.h"
#include "parser.h"

enum {
	MAXTAPSIZ = 1024 * 5,
};

void
usage(char *prog)
{
	char *usage = "[-r] [-h|-v] FILE [INPUT]";

	fprintf(stderr, "USAGE: %s %s\n", prog, usage);
	exit(EXIT_FAILURE);
}

int
main(int argc, char **argv)
{
	int ext, rtape;
	parerr ret;
	dtm *tm;
	parser *par;
	char opt, *fc, *fp, tp[MAXTAPSIZ];

	rtape = 0;
	while ((opt = getopt(argc, argv, "rhv")) != -1) {
		switch (opt) {
		case 'r':
			rtape = 1;
			break;
		case 'v':
			fprintf(stderr, "tmsim-"VERSION"\n");
			return 1;
		case 'h':
		default:
			usage(argv[0]);
		}
	}

	if (argc <= 1 || optind >= argc)
		usage(argv[0]);

	fp = argv[optind];
	if (!(fc = readfile(fp)))
		die("couldn't read from input file");
	par = newparser(fc);

	tm = newtm();
	if ((ret = parsetm(par, tm)) != PAR_OK) {
		strparerr(par, ret, fp, stdout);
		return 1;
	}

	if (argc <= 2 || ++optind >= argc)
		return 0;

	free(fc);
	freeparser(par);

	writetape(tm, argv[optind]);
	ext = (runtm(tm)) ? 1 : 0;

	if (rtape) {
		readtape(tm, tp);
		fprintf(stdout, "TAPE: %s\n", tp);
	}

	return ext;
}
