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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "util.h"

/**
 * Compares the first 'n' bytes of two strings (like strncmp(3)).
 * However, unlike strncmp(3) it returns the position of the
 * first character that differed.
 *
 * @param s1 First string to use for comparison.
 * @param s2 Second string to use for comparison.
 * @param n Amount of bytes to compare.
 * @returns -1 If the two strings are equal, otherwise the position
 * 	of the first character that differed is returned.
 */
int
xstrncmp(char *s1, char *s2, size_t n)
{
	int i;

	i = 1;
	while (*s1 && *s1 == *s2 && i < n) {
		s1++;
		s2++;
		i++;
	}

	return (*s1 == *s2) ? -1 : i;
}

/**
 * Reads the content of the given file at the given path and returns a
 * pointer to a newly allocated string containing the content.
 *
 * @param Path to file which should be read.
 * @returns Pointer to a string containing the content of the given file.
 * 	If an error occured while trying to read the given file NULL is
 * 	returned and errno is set to indicate the error.
 */
char*
readfile(char *fp)
{
	FILE *fd;
	char *fc;
	struct stat st;

	if (stat(fp, &st))
		return NULL;
	else
		fc = emalloc(st.st_size + 1);

	if (!(fd = fopen(fp, "r")))
		return NULL;
	if (fread(fc, sizeof(char), st.st_size, fd) != st.st_size)
		return NULL;

	if (fclose(fd))
		return NULL;

	fc[st.st_size] = '\0';
	return fc;
}



/**
 * Calls estrndup(3) but terminates the program EXIT_FAILURE if strndup returned
 * an error.
 *
 * @param s Pointer to a string which should be duplicated.
 * @param n Amount of bytes to copy.
 * @returns Pointer to a new string which is a duplicate of the given one.
 */
char*
estrndup(char *s, size_t n) {
	char *r;

	if (!(r = strndup(s, n)))
		die("strndup failed");

	return r;
}

/**
 * Calls malloc(3) but terminates the program with EXIT_FAILURE if malloc
 * returned an error.
 *
 * @param size Amount of memory (in bytes) which should be allocated.
 * @returns Pointer to the allocated memory.
 */
void*
emalloc(size_t size)
{
	void *r;

	if (!(r = malloc(size)))
		die("malloc failed");

	return r;
}

/**
 * Calls pthread_spin_lock(3) but terminates the program with EXIT_FAILURE if
 * pthread_spin_lock returned an error.
 *
 * @param lock Spin lock which should be locked.
 */
void
pthread_spin_elock(pthread_spinlock_t *lock)
{
	if (pthread_spin_lock(lock))
		die("pthread_spin_lock failed");
}

/**
 * Calls pthread_spin_unlock(3) but terminates the program with EXIT_FAILURE if
 * pthread_spin_unlock returned an error.
 *
 * @param lock Spin lock which should be unlocked.
 */
void
pthread_spin_eunlock(pthread_spinlock_t *lock)
{
	if (pthread_spin_unlock(lock))
		die("pthread_spin_unlock failed");
}

/**
 * Calls sem_wait(3) but terminates the program with EXIT_FAILURE if sem_wait
 * returned an error.
 *
 * @param sem Semaphore to call sem_wait on.
 */
void
sem_ewait(sem_t *sem)
{
	if (sem_wait(sem))
		die("sem_wait failed");
}

/**
 * Calls sem_post(3) but terminates the program with EXIT_FAILURE if sem_post
 * returned an error.
 *
 * @param sem Semaphore to call sem_post on.
 */
void
sem_epost(sem_t *sem)
{
	if (sem_post(sem))
		die("sem_post failed");
}
