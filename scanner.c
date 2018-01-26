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

#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include <pthread.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <semaphore.h>
#include <stddef.h>

#include "util.h"
#include "token.h"
#include "queue.h"
#include "scanner.h"
#include "turing.h"

static void lexspace(scanner*);
static void lexcomment(scanner*);
static void lexstate(scanner*);
static void lexterm(scanner*);

/**
 * Reads the next character from the input string and increments current
 * position of the scanner in the input string.
 *
 * @param scr Scanner from which character should be read.
 * @returns The next character or -1 if there is none.
 */
static signed char
nextch(scanner *scr)
{
	if (scr->pos >= scr->inlen)
		return -1;

	return scr->input[scr->pos++];
}

/**
 * Reads the next character from the input string but doesn't increment
 * the current position of the scanner.
 *
 * @param scr Scanner from which character should be read.
 * @returns The next character or -1 if there is none.
 */
static signed char
peekch(scanner *scr)
{
	signed char nxt;
	
	nxt = nextch(scr);
	if (nxt != -1)
		scr->pos--;
	return nxt;
}

/**
 * Ignores all characters in the input string after the current start position.
 *
 * @param scr Scanner for which characters should be ignored.
 */
static void
ignore(scanner *scr)
{
	scr->start = scr->pos;
}

/**
 * Whether or not the current character is a valid turing machine input
 * symbol. Which is the case if it is either an alphanumeric character
 * or the special blank character.
 *
 * @param c Character which should be examined.
 * @returns Non-zero integer if it is, zero if it isn't.
 */
static int
issymbol(char c)
{
	return isalnum(c) || c == BLANKCHAR;
}

/**
 * Emits a new token and enqueues it.
 *
 * @param scr Scanner which found the token that should be emitted.
 * @param tkt Type of the token.
 * @param value Value of the token, should be greater than or equal to zero.
 */
static void
emit(scanner *scr, toktype tkt, unsigned char value)
{
	token *tok;

	tok = emalloc(sizeof(token));
	tok->type = tkt;
	tok->line = scr->line;
	tok->column = scr->column;
	tok->value = value;

	enqueue(scr->tqueue, tok);
	scr->start = scr->pos;
}

/**
 * Tail recursive function parsing the entire input string.
 * Usually passed to pthread_create(3).
 *
 * @param scr Pointer to the associated scanner.
 */
static void
lexany(scanner *scr)
{
	signed char nxt;

	/* Make this function a cancel point. */
	pthread_testcancel();

	scr->column++;
	if ((nxt = nextch(scr)) == -1) {
		scr->column = 0;
		if (scr->line > 1)
			scr->line--;

		emit(scr, TOK_EOF, TOKNOP);
		LEXRET(scr, NULL);
	}

	switch (nxt) {
	case '\n':
		scr->column = 0;
		scr->line++;
		ignore(scr);
		LEXRET(scr, lexany);
	case '#':
		LEXRET(scr, lexcomment);
	case ',':
		emit(scr, TOK_COMMA, TOKAUTO(scr));
		LEXRET(scr, lexany);
	case ';':
		emit(scr, TOK_SEMICOLON, TOKAUTO(scr));
		LEXRET(scr, lexany);
	case '{':
		emit(scr, TOK_LBRACKET, TOKAUTO(scr));
		LEXRET(scr, lexany);
	case '}':
		emit(scr, TOK_RBRACKET, TOKAUTO(scr));
		LEXRET(scr, lexany);
	case '<':
		emit(scr, TOK_SMALLER, TOKAUTO(scr));
		LEXRET(scr, lexany);
	case '>':
		emit(scr, TOK_GREATER, TOKAUTO(scr));
		LEXRET(scr, lexany);
	case '|':
		emit(scr, TOK_PIPE, TOKAUTO(scr));
		LEXRET(scr, lexany);
	case 'q':
		if (isdigit(peekch(scr)))
			LEXRET(scr, lexstate);
	case '=':
		if (nextch(scr) == '>')
			emit(scr, TOK_NEXT, TOKNOP);
		else
			emit(scr, TOK_ERROR, ERR_UNKOWN);

		scr->column++;
		LEXRET(scr, lexany);
	}

	if (issymbol(nxt) && !issymbol(peekch(scr))) {
		emit(scr, TOK_SYMBOL, TOKAUTO(scr));
		LEXRET(scr, lexany);
	} else if (isspace(nxt)) {
		LEXRET(scr, lexspace);
	}

	LEXRET(scr, lexterm);
}

/**
 * Skips/Ignores all white-space characters. Also
 * calls lexany as soon as it encounters the first
 * non-white-space character.
 *
 * @param scr Pointer to the associated scanner.
 */
static void
lexspace(scanner *scr)
{
	while (isspace(peekch(scr))) {
		nextch(scr);
		scr->column++;
	}

	ignore(scr);
	LEXRET(scr, lexany);
}

/**
 * Skips/Ignores a comment. A comment begins with the
 * ASCII symbol '#' and ends with a newline.
 *
 * @param scr Pointer to the associated scanner.
 */
static void
lexcomment(scanner *scr) {
	while (peekch(scr) != '\n')
		if (nextch(scr) == -1)
			break;

	/* We don't need to increment scr->column here because
	 * comments are currently not exposed to the parser
	 * and since they end with a newline they don't effect
	 * other tokens either (unlike white-spaces). */

	ignore(scr);
	LEXRET(scr, lexany);
}

/**
 * Lexes a state name. Also calls lexany as soon as
 * it encounters a non-digit character. Besides an
 * error token is emitted if converting the token
 * name to an int would cause an integer over/underflow.
 *
 * @pre The previous char must be the ASCII character 'q'.
 * @param scr Pointer to the associated scanner.
 */
static void
lexstate(scanner *scr)
{
	char *input, buf[STATELEN];
	unsigned int col;
	size_t len;
	long value;

	input = &scr->input[scr->pos];

	col = scr->column;
	while (isdigit(peekch(scr)))
		nextch(scr);
	scr->column = col;

	len = scr->pos - scr->start - 1;
	if (len > STATELEN - 1) {
		emit(scr, TOK_ERROR, ERR_OVERFLOW);
		goto ret;
	}

	strncpy(buf, input, len);
	buf[len] = '\0';

	value = strtol(buf, NULL, 10);
	if (value < 0)
		emit(scr, TOK_ERROR, ERR_UNDERFLOW);
	else if (value > UCHAR_MAX)
		emit(scr, TOK_ERROR, ERR_OVERFLOW);
	else
		emit(scr, TOK_STATE, (unsigned char)value);

ret:
	scr->column += (unsigned int)len; /* Initial 'q' and digits. */
	LEXRET(scr, lexany);
}

/**
 * Attempts to lex the terminal symbols `start:` and `accept:`. It calls
 * ::lexany as soon as it reaches the end of the given terminal symbol
 * or if a character is encountered which is not part of the any known
 * terminal symbol.
 *
 * @param scr Pointer to the associated scanner.
 */
static void
lexterm(scanner *scr)
{
	char *ter;
	toktype tkt;
	size_t pos, len;

	switch (scr->input[scr->start]) {
	case 's':
		tkt = TOK_START;
		ter = "start:";
		break;
	case 'a':
		tkt = TOK_ACCEPT;
		ter = "accept:";
		break;
	default:
		emit(scr, TOK_ERROR, ERR_UNKOWN);
		LEXRET(scr, lexany);
	}

	len = strlen(ter);
	if (!xstrncmp(ter, &scr->input[scr->start], len, &pos)) {
		emit(scr, tkt, TOKNOP);
	} else {
		/* pos should always be < strlen({start:,accept:}) */
		assert(pos <= UINT_MAX - 1);

		scr->column += (unsigned int)pos;
		emit(scr, TOK_ERROR, ERR_UNEXPECTED);
	}

	scr->pos += --len;
	scr->start = scr->pos;

	scr->column += (unsigned int)len;
	LEXRET(scr, lexany);
}

/**
 * Function invoked by a seperate LWP for parsing the input string. This
 * function simply calls the current ::statefn until the called
 * ::statefn function sets the current state to `NULL`.
 *
 * @param pscr Void pointer to the scanner used for lexing.
 * @returns A null pointer.
 */
static void*
tokloop(void *pscr)
{
	scanner *scr;

	scr = (scanner*)pscr;
	while (scr->state != NULL)
		(*scr->state)(scr); /* fn must set scr->state. */

	return NULL;
}

/**
 * Creates a new scanner for the given input string and starts lexing the
 * string in a seperated thread.
 *
 * @param input Input string which should be scanned.
 * @returns Scanner for the given input string.
 */
scanner*
scanstr(char *input)
{
	scanner *scr;

	scr = emalloc(sizeof(scanner));
	scr->tqueue = newqueue();
	scr->state = lexany;
	scr->pos = scr->start = scr->column = 0;
	scr->inlen = strlen(input);
	scr->input = input;
	scr->line = 1;

	if (pthread_create(&scr->thread, NULL, tokloop, (void*)scr))
		die("pthread_create failed");

	return scr;
}

/**
 * Frees the allocated memory for the given scanner.
 *
 * @param scr Scanner for which allocated memory should be freed.
 */
void
freescanner(scanner *scr)
{
	assert(scr);

	if (!pthread_cancel(scr->thread)) {
		if (pthread_join(scr->thread, NULL))
			die("pthread_join failed");
	}

	freequeue(scr->tqueue);
	free(scr);
}

/**
 * Returns the least recent token scanned by the given scanner. If no
 * token has been scanned so for this function blocks until a token has
 * been emitted by the scanner. If the last token (TOK_EOF) was already
 * returned by this function then it causes a deadlock.
 *
 * @pre A previous call of this function didn't return TOK_EOF.
 * @param scr Scanner to extract token from.
 */
token*
nexttoken(scanner *scr)
{
	return dequeue(scr->tqueue);
}
