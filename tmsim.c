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
#include <string.h>
#include <libgen.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "util.h"
#include "token.h"
#include "queue.h"
#include "scanner.h"
#include "turing.h"
#include "parser.h"

static char *prog;

void
exporttrans(tmtrans *trans, tmstate *state, void *arg)
{
	FILE *stream = (FILE*)arg;

	fprintf(stream, "q%d -> q%d [label=\"%c/%c/%c\"];\n",
		state->name, trans->nextstate,
		trans->rsym, trans->wsym,
		dirstr(trans->headdir));
}

void
exportstate(tmstate *state, void *arg)
{
	FILE *stream = (FILE*)arg;

	eachtrans(state, exporttrans, stream);
}

void
export(dtm *tm, FILE *stream)
{

	fprintf(stream, "digraph G {\nrankdir = \"LR\";\n\n");

	fprintf(stream, "node [shape = %s];\n", "diamond");
	fprintf(stream, "q%d;\n", tm->start);

	fprintf(stream, "\nnode [shape = %s];\n", "doublecircle");
	for (int i = 0; i < tm->acceptsiz; i++)
		fprintf(stream, "q%d;\n", tm->accept[i]);

	fprintf(stream, "\nnode [shape = %s];\n", "circle");
	eachstate(tm, exportstate, stream);
	fprintf(stream, "}\n");
}

char*
readfile(char *fp)
{
	FILE *fd;
	char *fc;
	struct stat st;

	if (stat(fp, &st))
		return NULL;
	else
		fc = emalloc(st.st_size + 1);

	if (!(fd = fopen(fp, "r")))
		return NULL;
	if (fread(fc, sizeof(char), st.st_size, fd) != st.st_size)
		return NULL;

	if (fclose(fd))
		return NULL;

	fc[st.st_size] = '\0';
	return fc;
}

void
usage(void)
{
	if (!strcmp(prog, "tmsim-export"))
		fprintf(stderr, "USAGE: %s FILE\n", prog);
	else
		fprintf(stderr, "USAGE: %s FILE INPUT\n", prog);

	exit(EXIT_FAILURE);
}

int
main(int argc, char **argv)
{
	parerr ret;
	dtm *tm;
	parser *par;
	char *fc, *fp;

	prog = basename(argv[0]);
	if (argc <= 1)
		usage();

	fp = argv[1];
	if (!(fc = readfile(fp)))
		die("couldn't read from input file");
	par = newparser(fc);

	tm = newtm();
	if ((ret = parsetm(par, tm)) != PAR_OK) {
		strparerr(par, ret, fp, stdout);
		return 1;
	}

	free(fc);
	freeparser(par);

	if (!strcmp(prog, "tmsim-export")) {
		export(tm, stdout);
		return 0;
	} else if (argc <= 2) {
		usage();
	}

	writetape(tm, argv[2]);
	if (runtm(tm))
		return 1;
	else
		return 0;
}
