/*
 * Copyright © 2016-2018 Sören Tempel
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

/**
 * Can be used in conjunction with emit if the character in the input
 * string at the start position of this token should be used as the
 * value field of the enqueued token.
 *
 * @param SCANNER Scanner for which a value should be emitted.
 */
#define TOKAUTO(SCANNER) \
	((unsigned char)SCANNER->input[scr->start])

/**
 * Macro which must be used to return from a ::scanfn function. This
 * macro sets the state field of given scanner and returns afterwards.
 *
 * @param SCANNER Scanner for which state field should be modified.
 * @param FUNCTION Function pointer which should be used as the next
 * 	state.
 */
#define LEXRET(SCANNER, FUNCTION) \
	do { SCANNER->state = FUNCTION; return; } while (0)

enum {
	/**
	 * Passed to emit (as value) if the token doesn't have a
	 * meaningful character value.
	 */
	TOKNOP = 0,

	/**
	 * Amount of bytes needed for a string representation of the
	 * ::tmname type plus one byte for null termination.
	 */
	STATELEN = 4,
};

/**
 * Multithreaded lexer for the tmsim input files.
 */
typedef struct _scanner scanner;

/**
 * Function pointer representing the current state of a ::scanner.
 */
typedef void (*scanfn)(scanner *scr);

struct _scanner {
	/**
	 * Current state of the scanner. This field points to the
	 * function which should be invoked next for parsing the
	 * upcoming characters.
	 */
	scanfn state;

	/**
	 * Thread used to perform lexical scanning of the input string.
	 */
	pthread_t thread;

	/**
	 * Queue to which recognized tokens should be added.
	 */
	queue *tqueue;

	char *input;		/**< Input string passed to ::scanstr. */
	size_t inlen;		/**< Length of the input string. */

	size_t pos;		/**< Current position in the input string. */
	size_t start;		/**< Start position of current token in input string. */

	unsigned int line;	/**< Line being analyzed currently. */
	unsigned int column;	/**< Column being analyzed currently. */
};

scanner *scanstr(char*);
token *nexttoken(scanner*);
void freescanner(scanner*);
