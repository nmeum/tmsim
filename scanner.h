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

enum {
	/**
	 * Passed to emit (as value) if the character in the input string at
	 * the start position of this token should be used as the value field
	 * of the enqueued token.
	 */
	TOKAUTO = -1,

	/**
	 * Passed to emit (as value) if the token doesn't have a meaningful
	 * character value.
	 */
	TOKNOP = -2,
};

/**
 * Multithreaded lexer for the tmsim input files.
 */
typedef struct _scanner scanner;

struct _scanner {
	/**
	 * Thread used to perform lexical scanning of the input string.
	 */
	pthread_t *thread;

	/**
	 * Queue to which recognized tokens should be added.
	 */
	queue *tqueue;

	size_t pos;	/**< Current position in the input string. **/
	size_t start;	/**< Start position of current token in input string. */
	size_t inlen;	/**< Length of the input string. */

	char *input;	/**< Input string passed to scanstr. */

	int line;	/**< Line number the scanner is currently analyzing. */
	int column;	/**< Column the scanner is currently analyzing. */
};

scanner *scanstr(char*);
token *nexttoken(scanner*);
void freescanner(scanner*);
char* linenum(scanner*, int);
