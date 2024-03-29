/*
 * Copyright © 2016-2019 Sören Tempel
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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>

#include "parser.h"
#include "scanner.h"
#include "token.h"
#include "turing.h"
#include "util.h"

/**
 * Macro which should be used if the next expected token is a semicolon token.
 * If it isn't the proper error code is returned.
 */
#define EXPSEM(T) \
	do { \
		if (T->type != TOK_SEMICOLON) \
			return PAR_SEMICOLON; \
	} while (0)

/**
 * Returns the next token and advances the parser position.
 *
 * @param par Parser to extract next token from.
 * @pre A previous call of this function should not have returned TOK_EOF.
 * @returns Next token.
 */
static token *
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
static token *
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
 * @param str Input which should be parsed.
 * @param len Length of the input.
 * @returns Pointer to the newly created parser.
 */
parser *
newparser(char *str, size_t len)
{
	parser *par;

	par = emalloc(sizeof(parser));
	par->scr = scanstr(str, len);
	par->peektok = par->prevtok = par->tok = NULL;
	return par;
}

/**
 * Formats a parerr as a string and includes line and column information
 * where the error (presumably) ocurred. The string is directly written
 * to the given stream.
 *
 * @pre Parser must have encountered an error.
 * @param par Parser which returned the parerr.
 * @param err Parser error returned by ::parsetm.
 * @param fn Name of the file the parser was trying to parse.
 * @param stream Stream to write error message to.
 * @return Number of characters written to the stream.
 */
int
strparerr(parser *par, parerr err, char *fn, FILE *stream)
{
	int r;
	size_t pos;
	token *tok;
	char *msg, *line, *marker;

	assert(err != PAR_OK);
	tok = par->tok;
	assert(tok);
	msg = "Unkown error.";

	/* Check for scanner error. */
	if (tok->type == TOK_ERROR) {
		switch (tok->value) {
		case ERR_OVERFLOW:
			msg = "Numeric state name exceeds INT_MAX.";
			goto ret;
		case ERR_UNDERFLOW:
			msg = "Numeric state names can't be negative.";
			goto ret;
		case ERR_UNKOWN:
			msg = "Lexer encountered an unknown character.";
			goto ret;
		case ERR_UNEXPECTED:
			msg = "A terminal string was expected but the "
			      "lexer encountered a character which is "
			      "not part of the expected string. Perhaps "
			      "you misspelled 'start:' or 'accept:'.";
			goto ret;
		}
	}

	/* Wasn't a lexer error so the err parameter is actually relevant. */
	switch (err) {
	case PAR_SEMICOLON:
		msg = "Missing semicolon, maybe the previous transition "
		      "is missing a semicolon or a metadata information "
		      "was not properly terminated with a semicolon.";
		break;
	case PAR_STATEDEFTWICE:
		msg = "This state was already defined previously. You can't "
		      "define states twice please move all transitions from "
		      "this state definition to the previous definition.";
		break;
	case PAR_TRANSDEFTWICE:
		msg = "Only deterministic turing machines are supported. "
		      "Meaning you can't have more than one transition "
		      "for the same input symbol.";
		break;
	case PAR_STARTKEY:
		msg = "An initial state wasn't defined. Please define it "
		      "using the 'start:' keyword.";
		return fprintf(stream, "%s: %s\n", fn, msg);
	case PAR_INITALSTATE:
		msg = "The initial state value cannot be left empty.";
		break;
	case PAR_ACCEPTKEY:
		msg = "Accepting states where not defined. Please define "
		      "one or more accepting states using the "
		      "'accept:' keyword.";
		return fprintf(stream, "%s: %s\n", fn, msg);
	case PAR_NONSTATEACCEPT:
		msg = "Your accepting state list contains a token which is "
		      "not a state name or is empty.";
		break;
	case PAR_STATEDEF:
		msg = "Expected a state definition but didn't find a valid "
		      "state name. Valid state names must match the "
		      "following regex: 'q[0-9]+'.";
		break;
	case PAR_LBRACKET:
		msg = "The parser expected an opening curly bracket as a "
		      "part of this state definition.";
		break;
	case PAR_RBRACKET:
		msg = "The parser expected a closing curly bracket as a "
		      "part of this state definition.";
		break;
	case PAR_RSYMBOL:
		msg = "Your transition definition is missing a symbol "
		      "which triggers this transition. This symbol "
		      "can only be an alphanumeric character or the "
		      "special blank character.";
		break;
	case PAR_DIRECTION:
		msg = "Expected direction to move head to, this symbol is "
		      "not a valid head direction symbol.";
		break;
	case PAR_WSYMBOL:
		msg = "Your transition definition is missing a symbol "
		      "which is written to the tape when this transition "
		      "is performed. This symbol can only be an "
		      "alphanumeric character or the special blank character.";
		break;
	case PAR_NEXTSTATESYM:
		msg = "The next state symbol ('=>') was expected but "
		      "not found.";
		break;
	case PAR_NEXTSTATE:
		msg = "Your transition is missing a state to transit to "
		      "when performing this transition.";
		break;
	case PAR_OK:
		/* Never reached */
		break;
	}

ret:
	if (!(line = linenum(par->scr->input, tok->line))) {
		msg = "Current token contains an invalid line number. "
		      "This is a bug, please consider reporting it.";
		return fprintf(stream, "%s\n", msg);
	}

	if (tok->type == TOK_EOF)
		pos = endofline(line);
	else
		pos = tok->column - 1;

	marker = mark(pos, line);
	r = fprintf(stream, "%s:%d:%d: %s\n %s\n %s\n", fn, tok->line,
	            tok->column, msg, line, marker);

	free(line);
	free(marker);
	return r;
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

	/* Tokens are freed in the next method. */

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
static parerr
parsemeta(parser *par, dtm *dest)
{
	par->tok = next(par);
	if (par->tok->type != TOK_START)
		return PAR_STARTKEY;

	par->tok = next(par);
	if (par->tok->type != TOK_STATE)
		return PAR_INITALSTATE;
	dest->start = par->tok->value;

	par->tok = next(par);
	EXPSEM(par->tok);

	par->tok = next(par);
	if (par->tok->type != TOK_ACCEPT)
		return PAR_ACCEPTKEY;

	do {
		par->tok = next(par);
		if (par->tok->type != TOK_STATE)
			return PAR_NONSTATEACCEPT;
		addaccept(dest, par->tok->value);

		par->tok = next(par);
	} while (par->tok->type == TOK_COMMA &&
	         par->tok->type != TOK_SEMICOLON);

	EXPSEM(par->tok);

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
 * @param dest Pointer to a transition, if the transition was parsed
 * 	successfully, the struct fields are initialized accordingly.
 * @return Error code or PAR_OK if no error was encountered.
 */
static parerr
parsetrans(parser *par, tmtrans *dest)
{
	par->tok = next(par);
	if (par->tok->type != TOK_SYMBOL)
		return PAR_RSYMBOL;
	dest->rsym = (char)par->tok->value;

	par->tok = next(par);
	switch (par->tok->type) {
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

	par->tok = next(par);
	if (par->tok->type != TOK_SYMBOL)
		return PAR_WSYMBOL;
	dest->wsym = (char)par->tok->value;

	par->tok = next(par);
	if (par->tok->type != TOK_NEXT)
		return PAR_NEXTSTATESYM;

	par->tok = next(par);
	if (par->tok->type != TOK_STATE)
		return PAR_NEXTSTATE;
	dest->nextstate = par->tok->value;

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
static parerr
parsestate(parser *par, tmstate *dest)
{
	tmtrans *trans;
	parerr ret;

	par->tok = next(par);
	if (par->tok->type != TOK_STATE)
		return PAR_STATEDEF;
	dest->name = par->tok->value;

	par->tok = next(par);
	if (par->tok->type != TOK_LBRACKET)
		return PAR_LBRACKET;

	while (peek(par)->type != TOK_RBRACKET) {
		trans = emalloc(sizeof(tmtrans));
		if ((ret = parsetrans(par, trans)) != PAR_OK) {
			free(trans);
			return ret;
		}

		if (addtrans(dest, trans)) {
			free(trans);
			return PAR_TRANSDEFTWICE;
		}

		par->tok = next(par);
		EXPSEM(par->tok);
	}

	par->tok = next(par);
	if (par->tok->type != TOK_RBRACKET)
		return PAR_RBRACKET; /* Never reached. */

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
static parerr
parsestates(parser *par, dtm *dest)
{
	parerr ret;
	tmstate *state;

	while (peek(par)->type != TOK_EOF) {
		state = newtmstate();
		if ((ret = parsestate(par, state)) != PAR_OK) {
			free(state);
			return ret;
		}

		if (addstate(dest, state)) {
			free(state);
			return PAR_STATEDEFTWICE;
		}
	}

	/* skip and free EOF */
	par->tok = next(par);
	freetoken(par->tok);

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
