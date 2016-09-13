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

void *lexspace(scanner*);
void *lexcomment(scanner*);
void *lexstate(scanner*);
void *lexterm(scanner*, char*, toktype);

/**
 * Reads the next character from the input string and increments current
 * position of the scanner in the input string.
 *
 * @param scr Scanner from which character should be read.
 * @returns The next character or -1 if there is none.
 */
char
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
char
peekch(scanner *scr)
{
	char nxt = nextch(scr);
	if (nxt != -1)
		scr->pos--;
	return nxt;
}

/**
 * Ignores all characters in the input string after the current start position.
 *
 * @param scr Scanner for which characters should be ignored.
 */
void
ignore(scanner *scr)
{
	scr->start = scr->pos;
}

/**
 * Whether or not the current character is a valid turing maschine input symbol.
 * Which is the case if it is either an alphanumeric character or a digit.
 *
 * @returns Non-zero integer if it is, zero if it isn't.
 */
int
issymbol(char c)
{
	return isalpha(c) || isdigit(c) || c == BLANKCHAR;
}

/**
 * Emits a new token and enqueues it.
 *
 * @param scr Scanner which found the token that should be emitted.
 * @param tkt Type of the token.
 * @param value Value of the token, should be greater than or equal to zero.
 */
void
emit(scanner *scr, toktype tkt, int value)
{
	token *tok;

	tok = emalloc(sizeof(token));
	tok->type = tkt;
	tok->line = scr->line;
	tok->column = scr->column;
	tok->value = (value == TOKAUTO) ? scr->input[scr->start] : value;

	enqueue(scr->tqueue, tok);
	scr->start = scr->pos;
}

/**
 * Tail recursive function parsing the entire input string.
 * Usually passed to pthread_create(3).
 *
 * @param pscr Void pointer to the scanner used for lexing.
 * @returns A null pointer.
 */
void*
lexany(void *pscr)
{
	char nxt;
	scanner *scr = (scanner*)pscr;

	scr->column++;
	if ((nxt = nextch(scr)) == -1) {
		emit(scr, TOK_EOF, TOKNOP);
		return NULL;
	}

	switch (nxt) {
	case '\n':
		scr->column = 0;
		scr->line++;
		ignore(scr);
		return lexany(scr);
	case '#':
		return lexcomment(scr);
	case ',':
		emit(scr, TOK_COMMA, TOKAUTO);
		return lexany(scr);
	case ';':
		emit(scr, TOK_SEMICOLON, TOKAUTO);
		return lexany(scr);
	case '{':
		emit(scr, TOK_LBRACKET, TOKAUTO);
		return lexany(scr);
	case '}':
		emit(scr, TOK_RBRACKET, TOKAUTO);
		return lexany(scr);
	case '<':
		emit(scr, TOK_SMALLER, TOKAUTO);
		return lexany(scr);
	case '>':
		emit(scr, TOK_GREATER, TOKAUTO);
		return lexany(scr);
	case '|':
		emit(scr, TOK_PIPE, TOKAUTO);
		return lexany(scr);
	case 'q':
		if (isdigit(peekch(scr)))
			return lexstate(scr);
	case '=':
		if (nextch(scr) == '>')
			emit(scr, TOK_NEXT, TOKNOP);
		else
			emit(scr, TOK_ERROR, ERR_UNKOWN);

		scr->column++;
		return lexany(scr);
	}

	if (issymbol(nxt) && !issymbol(peekch(scr))) {
		emit(scr, TOK_SYMBOL, TOKAUTO);
		return lexany(scr);
	} else if (isspace(nxt)) {
		return lexspace(scr);
	}

	switch (nxt) {
	case 's':
		return lexterm(scr, "start:", TOK_START);
	case 'a':
		return lexterm(scr, "accept:", TOK_ACCEPT);
	default:
		emit(scr, TOK_ERROR, ERR_UNKOWN);
		return lexany(scr);
	}
}

/**
 * Skips/Ignores all white-space characters. Also
 * calls lexany as soon as it encounters the first
 * non-white-space character.
 *
 * @param scr Pointer to the associated scanner.
 * @returns A null pointer.
 */
void*
lexspace(scanner *scr)
{
	while (isspace(peekch(scr))) {
		nextch(scr);
		scr->column++;
	}

	ignore(scr);
	return lexany(scr);
}

/**
 * Skips/Ignores a comment. A comment begins with the
 * ASCII symbol '#' and ends with a newline.
 *
 * @param scr Pointer to the associated scanner.
 * @returns A null pointer.
 */
void*
lexcomment(scanner *scr) {
	while (peekch(scr) != '\n') {
		nextch(scr);

		/* We don't need to increment scr->column here because
		 * comments are currently not exposed to the parser
		 * and since they end with a newline they don't effect
		 * other tokens either (unlike white-spaces). */
	}

	ignore(scr);
	return lexany(scr);
}

/**
 * Lexes a state name. Also calls lexany as soon as
 * it encounters a non-digit character. Besides an
 * error token is emitted if converting the token
 * name to an int would cause an integer over/underflow.
 *
 * @pre The previous char must be the ASCII character 'q'.
 * @param scr Pointer to the associated scanner.
 * @returns A null pointer.
 */
void*
lexstate(scanner *scr)
{
	int value, col;
	size_t len;

	col = scr->column;
	while (isdigit(peekch(scr)))
		nextch(scr);
	scr->column = col;

	len = scr->pos - scr->start;
	char dest[len];

	strncpy(dest, &scr->input[scr->pos - 1], len);
	dest[len] = '\0';

	value = strtol(dest, NULL, 10);
	if (value == LONG_MIN)
		emit(scr, TOK_ERROR, ERR_UNDERFLOW);
	else if (value == LONG_MAX)
		emit(scr, TOK_ERROR, ERR_OVERFLOW);

	emit(scr, TOK_STATE, value);
	scr->column += len - 1;

	return lexany(scr);
}

/**
 * Lexes a given terminal symbol. Also calls lexany as soon as it reaches the
 * end of the given terminal symbol or a character which is not part of the
 * given terminal symbol.
 *
 * @param scr Pointer to the associated scanner.
 * @param ter Expected terminal symbol.
 * @param tkt Token type to use if the expected terminal symbol was found.
 * @returns A null pointer.
 */
void*
lexterm(scanner *scr, char *ter, toktype tkt)
{
	int ret;
	size_t len;

	len = strlen(ter);
	if ((ret = xstrncmp(ter, &scr->input[scr->start], len)) == -1) {
		emit(scr, tkt, TOKNOP);
	} else {
		scr->column = ret;
		emit(scr, TOK_ERROR, ERR_UNEXPECTED);
	}

	scr->pos += len;
	scr->start = scr->pos;

	scr->column += len;
	return lexany(scr);
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
	scr->thread = emalloc(sizeof(pthread_t));
	scr->tqueue = newqueue();
	scr->pos = scr->start = scr->column = 0;
	scr->inlen = strlen(input);
	scr->input = input;
	scr->line = 1;

	if (pthread_create(scr->thread, NULL, lexany, (void*)scr))
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

	if (!pthread_cancel(*scr->thread))
		die("pthread_cancel failed");

	free(scr->thread);
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

/**
 * Returns the content of the line with the given line number.
 *
 * @pre Line must be greater than 0.
 * @param scr Scanner to extract line from.
 * @param line Line number.
 * @return -1 if the line number doesn't exist, 0 otherwise.
 */
char*
linenum(scanner *scr, int line)
{
	char *c1, *c2, *res;
	ptrdiff_t p1, p2;
	size_t len;

	assert(line >= 1);

	p1 = 0;
	while ((c1 = strchr(&scr->input[p1], '\n'))) {
		if (--line == 0) {
			if ((c2 = strchr(&scr->input[p1], '\n'))) {
				p2 = c2 - scr->input + 1;
				len = p2 - p1;
			} else {
				len = scr->inlen - p1;
			}

			res = estrndup(&scr->input[p1], len - 1);
			res[len - 1] = '\0'; /* Overwrite newline. */
			return res;
		}

		p1 = c1 - scr->input + 1;
	}

	return NULL;
}
