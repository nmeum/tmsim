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

/**
 * Shape used for normal states. Normal states are all states that are
 * neither an accepting nor an initial state.
 */
static char *nodeshape = "circle";

/**
 * Shape used for the initial node.
 */
static char *initialshape = "diamond";

/**
 * Shape used for the accepting nodes.
 */
static char *acceptingshape = "doublecircle";

/**
 * Writes the dot language reprasentation for a given transition
 * from a given state to a given stream.
 *
 * @param trans Transition to create dot markup for.
 * @param state State the transition belongs to.
 * @param arg Void pointer to a stream the output should be written to.
 */
static void
exporttrans(tmtrans *trans, tmstate *state, void *arg)
{
	FILE *stream = (FILE*)arg;

	fprintf(stream, "q%d -> q%d [label=\"%c/%c/%c\"];\n",
		state->name, trans->nextstate,
		trans->rsym, trans->wsym,
		dirstr(trans->headdir));
}

/**
 * Writes the dot language reprasentation for a given state
 * to a given stream.
 *
 * @param state State to create dot markup for.
 * @param arg Void pointer to a stream the output should be written to.
 */
static void
exportstate(tmstate *state, void *arg)
{
	FILE *stream = (FILE*)arg;

	eachtrans(state, exporttrans, stream);
}

/**
 * Writes the dot language reprasentation for a given turing machine
 * to a given stream.
 *
 * @param tm Turing machine to create dot markup for.
 * @param stream Stream to write dot markup to.
 */
static void
export(dtm *tm, FILE *stream)
{

	fprintf(stream, "digraph G {\nrankdir = \"LR\";\n\n");

	fprintf(stream, "node [shape = %s];\n", initialshape);
	fprintf(stream, "q%d;\n", tm->start);

	fprintf(stream, "\nnode [shape = %s];\n", acceptingshape);
	for (size_t i = 0; i < tm->acceptsiz; i++)
		fprintf(stream, "q%d;\n", tm->accept[i]);

	fprintf(stream, "\nnode [shape = %s];\n", nodeshape);
	eachstate(tm, exportstate, stream);
	fprintf(stream, "}\n");
}

/**
 * Writes the usage string for this program to stderr and terminates
 * the programm with EXIT_FAILURE.
 */
static void
usage(char *prog)
{
	char *usage = "[-s nodeshape] [-i initialshape]\n"
		"\t[-a acceptingshape] [-o path] [-h|-v] FILE";

	fprintf(stderr, "USAGE: %s %s\n", prog, usage);
	exit(EXIT_FAILURE);
}

/**
 * The main function invoked when the program is started.
 *
 * @param argc Amount of command line parameters.
 * @param argv Command line parameters.
 */
int
main(int argc, char **argv)
{
	int opt;
	parerr ret;
	dtm *tm;
	parser *par;
	char *fc, *fp;
	FILE *ofd = stdout;

	while ((opt = getopt(argc, argv, "s:i:a:o:hv")) != -1) {
		switch (opt) {
		case 's':
			nodeshape = optarg;
			break;
		case 'i':
			initialshape = optarg;
			break;
		case 'a':
			acceptingshape = optarg;
			break;
		case 'o':
			if (!(ofd = fopen(optarg, "w")))
				die("couldn't open output file");
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

	free(fc);
	freeparser(par);

	export(tm, ofd);
	return 0;
}
