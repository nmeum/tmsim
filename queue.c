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
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>

#include "queue.h"
#include "token.h"
#include "util.h"

/**
 * Allocates memory for a new queue and initializes it.
 *
 * @returns A pointer to the newly created queue.
 */
queue *
newqueue(void)
{
	queue *qu;

	qu = emalloc(sizeof(queue));
	qu->head = qu->tail = 0;

	if ((errno = pthread_mutex_init(&qu->hmtx, NULL)) ||
	    (errno = pthread_mutex_init(&qu->tmtx, NULL)))
		die("pthread_mutex_init failed");

	if (sem_init(&qu->fullsem, 0, 0) ||
	    sem_init(&qu->emptysem, 0, NUMTOKENS))
		die("sem_init failed");

	return qu;
}

/**
 * Enqueues a new token at the end of this queue.
 *
 * @pre Token must not be NULL.
 * @param qu Queue where token should be enqueued.
 * @param value Token which should be enqueued.
 */
void
enqueue(queue *qu, token *value)
{
	sem_ewait(&qu->emptysem);
	pthread_mutex_elock(&qu->tmtx);
	qu->tokens[qu->tail++ % NUMTOKENS] = value;
	pthread_mutex_eunlock(&qu->tmtx);
	sem_epost(&qu->fullsem);
}

/**
 * Dequeues the least recently added token from this queue.
 * Causes a deadlock if you already dequeued the last item from this queue.
 *
 * @returns Pointer to the least recently added token.
 */
token *
dequeue(queue *qu)
{
	token *ret;

	sem_ewait(&qu->fullsem);
	pthread_mutex_elock(&qu->hmtx);
	ret = qu->tokens[qu->head++ % NUMTOKENS];
	pthread_mutex_eunlock(&qu->hmtx);
	sem_epost(&qu->emptysem);

	return ret;
}

/**
 * Frees memory allocated for the given queue. Node values are not freed
 * you have to free those manually using the address returned on dequeue.
 *
 * @pre The last element of the queue was already dequeued (queue is empty)
 * 	besides the given queue should not be NULL.
 * @param qu Queue for which allocated memory should be freed.
 */
void
freequeue(queue *qu)
{
	assert(qu);

	if (sem_destroy(&qu->fullsem) || sem_destroy(&qu->emptysem))
		die("sem_destroy failed");

	if ((errno = pthread_mutex_destroy(&qu->hmtx)) ||
	    (errno = pthread_mutex_destroy(&qu->tmtx)))
		die("pthread_mutex_destroy failed");

	free(qu);
}
