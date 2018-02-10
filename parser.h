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

#ifndef _TMSIM_PARSER_H
#define _TMSIM_PARSER_H

#include <stdio.h>

#include "scanner.h"
#include "token.h"
#include "turing.h"

/**
 * A parser for the tmsim input format.
 */
typedef struct _parser parser;

struct _parser {
	/**
	 * Special token used to enable peeking functionality for tokens.
	 * Needed because we can't peek the next token from a concurrent queue.
	 */
	token *peektok;

	/**
	 * Previous token returned by a call to next. Used to free the previous
	 * token when requesting the next one.
	 */
	token *prevtok;

	/**
	 * Current token, stored here instead in order to extract line and
	 * column information form the token when the user requests an error
	 * string.
	 */
	token *tok;

	/**
	 * Underlying scanner for this parser.
	 */
	scanner *scr;
};

/**
 * Returned by various parser function to indicate what kind of error was
 * encountered. Makes it easier to debug syntax errors in input files.
 */
typedef enum {
	PAR_OK,        /**< Input was parsed successfully. */
	PAR_SEMICOLON, /**< Parser didn't encounter semicolon. */

	PAR_STATEDEFTWICE, /**< State was defined twice. */
	PAR_TRANSDEFTWICE, /**< Non-deterministic turing machine. */

	PAR_STARTKEY,    /**< Parser didn't find 'start:' keyword. */
	PAR_INITALSTATE, /**< Parser didn't find an initial state. */

	PAR_ACCEPTKEY,      /**< Parser didn't encounter 'accept:' keyword. */
	PAR_NONSTATEACCEPT, /**< Invalid token in accepting state list. */

	PAR_STATEDEF, /**< Expected statename for state definition. */
	PAR_LBRACKET, /**< Missing opening left bracket in state definition. */
	PAR_RBRACKET, /**< Missing closing right bracket in state definition. */

	PAR_RSYMBOL,      /**< Expected symbol to read for transition. */
	PAR_DIRECTION,    /**< Expected direction symbol for head movement. */
	PAR_WSYMBOL,      /**< Expected symbol to write after transition. */
	PAR_NEXTSTATESYM, /**< Expected '=>' symbol. */
	PAR_NEXTSTATE,    /**< Expected name of new state. */
} parerr;

parser *newparser(char *);
parerr parsetm(parser *, dtm *);
void freeparser(parser *);
int strparerr(parser *, parerr, char *, FILE *);

#endif
