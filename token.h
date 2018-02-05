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
 * Error codes used as token value when the value of the token type field is
 * equal to TOK_ERROR.
 */
typedef enum {
	ERR_OVERFLOW = 1,	/**< strtol(3) detected an integer overflow. */
	ERR_UNDERFLOW = 2,	/**< strtol(3) detected an integer underflow. */
	ERR_UNKOWN = 3,		/**< Lexer encountered an unknown character. */
	ERR_UNEXPECTED = 4,	/**< Lexer encountered an unexpected character. */
} errorcode;

/**
 * Valid values for the type field of the token struct.
 */
typedef enum {
	TOK_EOF,	/**< End of file. */
	TOK_ERROR,	/**< Error occured (see errorcode above). */

	TOK_START,	/**< Token specifies initial TM state. */
	TOK_ACCEPT,	/**< Token specifies accepting TM states. */
	TOK_NEXT,	/**< Token specifies next state in a transition. */

	TOK_SYMBOL,	/**< TM tape alphabet symbol. */
	TOK_STATE,	/**< TM state name, should match: 'q[0-9]+'. */

	TOK_COMMA,	/**< The ASCII ',' symbol. */
	TOK_SEMICOLON,	/**< The ASCII ';' symbol. */
	TOK_SMALLER,	/**< The ASCII '<' symbol. */
	TOK_GREATER,	/**< The ASCII '>' symbol. */
	TOK_LBRACKET,	/**< The ASCII '{' symbol */
	TOK_RBRACKET,	/**< The ASCII '}' symbol */
	TOK_PIPE,	/**< The ASCII '|' symbol. */
} toktype;

/**
 * Token data type used for successfully scanned input tokens.
 */
typedef struct _token token;

struct _token {
	toktype type;		/**< Type of this token (see above). */
	unsigned char value;	/**< Value of this token. */

	unsigned int line;	/**< Line of token in input file. */
	unsigned int column;	/**< Column of token in input file. */
};

void freetoken(token*);
