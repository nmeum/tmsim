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
#include <assert.h>
#include <stdio.h>
#include <semaphore.h>

#include "util.h"
#include "token.h"
#include "queue.h"

/**
 * Allocates memory for a new node and initializes it.
 *
 * @param value Value which should stored in this node.
 * @returns A pointer to the newly created node.
 */
node*
newnode(token *value)
{
	node *nd;

	nd = emalloc(sizeof(node));
	nd->value = value;
	nd->next  = NULL;
	return nd;
}

/**
 * Allocates memory for a new queue and initializes it.
 *
 * @returns A pointer to the newly created queue.
 */
queue*
newqueue(void)
{
	queue *qu;

	qu = emalloc(sizeof(queue));
	qu->head = qu->tail = newnode(NULL);
	qu->hlock = emalloc(sizeof(pthread_spinlock_t));
	qu->tlock = emalloc(sizeof(pthread_spinlock_t));

	if (pthread_spin_init(qu->hlock, PTHREAD_PROCESS_PRIVATE)
			|| pthread_spin_init(qu->tlock, PTHREAD_PROCESS_PRIVATE))
		die("pthread_spin_init failed");

	qu->fullsem  = emalloc(sizeof(sem_t));
	qu->emptysem = emalloc(sizeof(sem_t));

	if (sem_init(qu->fullsem, 0, 0)
			|| sem_init(qu->emptysem, 0, MAXNODES))
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
	node *nd;

	assert(value);
	nd = newnode(value);

	sem_ewait(qu->emptysem);
	pthread_spin_elock(qu->tlock);
	qu->tail->next = nd;
	qu->tail = nd;
	pthread_spin_eunlock(qu->tlock);
	sem_epost(qu->fullsem);
}

/**
 * Dequeues the least recently added token from this queue.
 * Causes a deadlock if you already dequeued the last item from this queue.
 *
 * @returns Pointer to the least recently added token.
 */
token*
dequeue(queue *qu)
{
	token *ret;
	node *nd, *newh;

	sem_ewait(qu->fullsem);
	pthread_spin_elock(qu->hlock);
	nd = qu->head;
	if (!(newh = nd->next)) {
		pthread_spin_eunlock(qu->hlock);
		return NULL;
	}

	ret = newh->value;
	qu->head = newh;
	pthread_spin_eunlock(qu->hlock);
	sem_epost(qu->emptysem);

	free(nd);
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

	if (qu->head) free(qu->head);
	if (sem_destroy(qu->fullsem) ||
			sem_destroy(qu->emptysem))
		die("sem_destroy failed");

	free(qu->fullsem);
	free(qu->emptysem);

	if (pthread_spin_destroy(qu->hlock)
			|| pthread_spin_destroy(qu->tlock))
		die("pthread_spin_destroy failed");

	free(qu->hlock);
	free(qu->tlock);
	free(qu);
}
