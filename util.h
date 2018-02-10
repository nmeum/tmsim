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

#ifndef _TMSIM_UTIL_H
#define _TMSIM_UTIL_H

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>

/**
 * Macro which calls perror(3) and terminates the program with EXIT_FAILURE.
 *
 * @param msg String passed as a first argument to perror(3).
 */
#define die(msg) \
	do { \
		perror(msg); \
		exit(EXIT_FAILURE); \
	} while (0)

int xstrncmp(char *, char *, size_t, size_t *);
char *readfile(char *);
char *mark(size_t, char *);

char *linenum(char *, unsigned int);
size_t endofline(char *);

char *estrndup(char *, size_t);
void *emalloc(size_t);
void *erealloc(void *, size_t);

void pthread_mutex_elock(pthread_mutex_t *);
void pthread_mutex_eunlock(pthread_mutex_t *);

void sem_ewait(sem_t *);
void sem_epost(sem_t *);

#endif
