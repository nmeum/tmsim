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

enum {
	NUMTOKENS = 5, /**< Amount of tokens kept in queue. */
};

/**
 * Two-lock concurrent queue algorithm. All operations wait for the queue to
 * become non-empty when retrieving an element, and wait for space to become
 * available in the queue when storing an element.
 */
typedef struct _queue queue;

struct _queue {
	token *tokens[NUMTOKENS]; /**< Array used to store tokens. */

	size_t head; /**< Index of the queue head. */
	size_t tail; /**< Index of the queue tail. */

	sem_t fullsem;  /**< Represents number of elements in the queue. */
	sem_t emptysem; /**< Represents number of empty places in the queue. */

	pthread_mutex_t hmtx; /**< Prevents concurrent access of head. */
	pthread_mutex_t tmtx; /**< Prevents concurrent access of tail. */
};

queue *newqueue(void);
void freequeue(queue*);
void enqueue(queue*, token*);
token *dequeue(queue*);
