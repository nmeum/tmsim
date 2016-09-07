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

#include "util.h"
#include "token.h"
#include "queue.h"
#include "scanner.h"
#include "turing.h"
#include "parser.h"

/**
 * Macro which should be used if the next expected token is a semicolon token.
 * If it isn't the proper error code is returned.
 */
#define EXPSEM(T) \
	do { if (T->type != TOK_SEMICOLON) \
		return PAR_SEMICOLON; \
	   } while(0)

/**
 * Returns the next token and advances the parser position.
 *
 * @param par Parser to extract next token from.
 * @pre A previous call of this function should not have returned TOK_EOF.
 * @returns Next token.
 */
token*
next(parser *par)
{
	token *tok;

	if (par->prevtok != NULL)
		freetoken(par->prevtok);

	tok = par->peektok;
	if (tok != NULL)
		par->peektok = NULL;
	else
		tok = nexttoken(par->scr);

	par->prevtok = tok;
	return tok;
}

/**
 * Returns the next token for the given parser without advancing the position.
 *
 * @pre A previous call of this ::next should not have returned TOK_EOF.
 * @param par Parser to extract next token from.
 * @returns Next token.
 */
token*
peek(parser *par)
{
	token *tok;

	tok = par->peektok;
	if (tok != NULL)
		return tok;

	par->peektok = nexttoken(par->scr);
	return par->peektok;
}

/**
 * Allocates memory for a new parser, initializes it and returns a
 * pointer to it.
 *
 * @param str String which should be parsed.
 * @returns Pointer to the newly created parser.
 */
parser*
newparser(char *str)
{
	parser *par;

	par = emalloc(sizeof(parser));
	par->scr = scanstr(str);
	par->peektok = par->prevtok = NULL;
	return par;
}

/**
 * Frees all resources for a given parser.
 *
 * @param par Pointer to the parser which should be freed.
 */
void
freeparser(parser *par)
{
	assert(par);

	freescanner(par->scr);
	free(par);
}

/**
 * Parses the metadata information of a tmsim input file.
 *
 * The following EBNF rules describes valid input:
 *
 * \code
 * metadata = "start:", statename, ";",
 * 	"accept:", statenames, ";";
 * \endcode
 *
 * @param par Parser for which metadata should be parsed.
 * @param dest Pointer to a Turing machine, if the metadata was
 * 	parsed successfully the accept array and the start field
 * 	of the dtm struct are set accordingly.
 * @return Error code or PAR_OK if no error was encountered.
 */
parerr
parsemeta(parser *par, dtm *dest)
{
	int i;
	token *tok;

	tok = next(par);
	if (tok->type != TOK_START)
		return PAR_STARTKEY;

	tok = next(par);
	if (tok->type != TOK_STATE)
		return PAR_INITALSTATE;
	dest->start = tok->value;

	EXPSEM(next(par));

	tok = next(par);
	if (tok->type != TOK_ACCEPT)
		return PAR_ACCEPTKEY;

	tok = next(par);
	for (i = 0; tok->type == TOK_STATE; tok = next(par)) {
		if (i > MAXACCEPT)
			return PAR_TOOMANYACCEPT;
		dest->accept[i++] = tok->value;

		tok = next(par);
		if (tok->type == TOK_SEMICOLON)
			break;
		else if (tok->type != TOK_COMMA)
			return PAR_MISSINGCOMMA;
	}

	if (i <= 0)
		return PAR_ACCEPTSTATE;
	else
		dest->acceptsiz = ++i;

	return PAR_OK;
}

/**
 * Parses a state transition of a tmsim input file.
 *
 * The following EBNF rules describes valid input:
 *
 * \code
 * transition = symbol, direction, symbol, "=>", statename;
 * \endcode
 *
 * @param par Parser for which a transition should be parsed.
 * @param rsym Input alphabet symbol which triggers this transition.
 * @param dest Pointer to a transition, if the transition was parsed
 * 	successfully, the struct fields are initialized accordingly.
 * @return Error code or PAR_OK if no error was encountered.
 */
parerr
parsetrans(parser *par, int *rsym, tmtrans *dest)
{
	token *tok;

	tok = next(par);
	if (tok->type != TOK_SYMBOL)
		return PAR_RSYMBOL;
	*rsym = tok->value;

	tok = next(par);
	switch (tok->type) {
	case TOK_SMALLER:
		dest->headdir = LEFT;
		break;
	case TOK_GREATER:
		dest->headdir = RIGHT;
		break;
	case TOK_PIPE:
		dest->headdir = STAY;
		break;
	default:
		return PAR_DIRECTION;
	}

	tok = next(par);
	if (tok->type != TOK_SYMBOL)
		return PAR_WSYMBOL;
	dest->symbol = tok->value;

	tok = next(par);
	if (tok->type != TOK_NEXT)
		return PAR_NEXTSTATESYM;

	tok = next(par);
	if (tok->type != TOK_STATE)
		return PAR_NEXTSTATE;
	dest->nextstate = tok->value;

	return PAR_OK;
}

/**
 * Parses a state definition of a tmsim input file.
 *
 * The following EBNF rules describes valid input:
 *
 * \code
 * state = statename, "{", [ transitions ], "}";
 * \endcode
 *
 * @param par Parser for which a state definition should be parsed.
 * @param dest Pointer to a Turing state, if the state definition was parsed
 * 	successfully, the struct fields are initialized accordingly.
 * @return Error code or PAR_OK if no error was encountered.
 */
parerr
parsestate(parser *par, tmstate *dest)
{
	int rsym;
	tmtrans *trans;
	parerr ret;
	token *tok;

	tok = next(par);
	if (tok->type != TOK_STATE)
		return PAR_STATEDEF;
	dest->name = tok->value;

	tok = next(par);
	if (tok->type != TOK_LBRACKET)
		return PAR_LBRACKET;

	while ((tok = peek(par))->type != TOK_RBRACKET) {
		trans = emalloc(sizeof(tmtrans));
		if ((ret = parsetrans(par, &rsym, trans) != PAR_OK)) {
			free(trans);
			return ret;
		}

		if (addtrans(dest, rsym, trans)) {
			free(trans);
			return PAR_TRANSDEFTWICE;
		}

		EXPSEM(next(par));
	}

	tok = next(par);
	if (tok->type != TOK_RBRACKET)
		return PAR_RBRACKET;

	return PAR_OK;
}

/**
 * Parses a series of state definitions of a tmsim input file.
 *
 * The following EBNF rules describes valid input:
 *
 * \code
 * states = { state };
 * \endcode
 *
 * @param par Parser for which a state definition should be parsed.
 * @param dest Pointer to a Turing machine, if the states were parsed
 * 	successfully, the struct fields are initialized accordingly.
 * @return Error code or PAR_OK if no error was encountered.
 */
parerr
parsestates(parser *par, dtm *dest)
{
	parerr ret;
	token *tok;
	tmstate *state;

	while ((tok = peek(par))->type != TOK_EOF) {
		state = newtmstate();
		if ((ret = parsestate(par, state)) != PAR_OK) {
			free(state);
			return ret;
		}

		if (addstate(dest, state))
			return PAR_STATEDEF;
	}

	/* skip and free EOF */
	tok = next(par);
	freetoken(tok);

	return PAR_OK;
}

/**
 * Parses a tmsim input file.
 *
 * The following EBNF rules describes valid input:
 *
 * \code
 * turingmachine = metadata, states;
 * \endcode
 *
 * @param par Parser for which a state definition should be parsed.
 * @param dest Pointer to a Turing machine, if the states were parsed
 * 	successfully, the struct fields are initialized accordingly.
 * @return Error code or PAR_OK if no error was encountered.
 */
parerr
parsetm(parser *par, dtm *dest)
{
	parerr ret;

	if ((ret = parsemeta(par, dest)) != PAR_OK)
		return ret;
	if ((ret = parsestates(par, dest)) != PAR_OK)
		return ret;

	return PAR_OK;
}
