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

#include <assert.h>
#include <stdlib.h>

#include "token.h"

/**
 * Frees memory allocated for the given token.
 *
 * @pre The given token should not be NULL.
 * @param tok Token for which allocated memory should be freed.
 */
void
freetoken(token *tok)
{
	assert(tok);

	free(tok);
}
