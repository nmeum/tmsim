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

/**
 * Macro which calls perror(3) and terminates the program with EXIT_FAILURE.
 *
 * @param msg String passed as a first argument to perror(3).
 */
#define die(msg) \
	do { perror(msg); exit(EXIT_FAILURE); } while (0)

int xstrncmp(char*, char*, size_t);
char *readfile(char*);
char *mark(int, char*);

char *estrndup(char *s, size_t n);
void *emalloc(size_t size);

void pthread_spin_elock(pthread_spinlock_t*);
void pthread_spin_eunlock(pthread_spinlock_t*);

void sem_ewait(sem_t*);
void sem_epost(sem_t*);
