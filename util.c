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

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "util.h"

/**
 * Returns a string which 'marks' the given position in the given
 * string. Meaning it returns a string which is excactly as long as the
 * first 'pos' characters of the given string when printed to a terminal
 * (tab characters will be retained. The last character (ignoring the
 * null byte) of this returned string is '^' and will thus point to the
 * character at 'pos' in the original string if the two string are
 * printed to a terminal seperated by a newline character.
 *
 * @pre The length of the given string must be equal or greater than pos.
 * @param pos Position of the character in the given string the
 * 	returned string should point to. The first element is at
 * 	position 0 not at position 1.
 * @param str String to create marker for.
 * @returns Marker for the given string as described above.
 */
char*
mark(size_t pos, char *str)
{
	char *res;

	res = estrndup(str, pos + 1);
	for (size_t i = 0; i <= pos; i++) {
		if (res[i] != '\t')
			res[i] = ' ';
	}

	res[pos] = '^';
	res[++pos] = '\0';

	return res;
}

/**
 * Returns the content of the line with the given line number.
 *
 * @param str Input string containing line.
 * @param line Line number.
 * @return A pointer to a dynamically allocated string containing
 * 	the content of the given line if it exists. NULL if it doesn't.
 */
char*
linenum(char *str, unsigned int line)
{
	size_t len;
	char ch, *res;
	size_t end, prev, newline;

	if (line < 1)
		return NULL;
	len = strlen(str);

	newline = prev = 0;
	for (end = 0; end <= len; end++) {
		ch = str[end];
		if (ch != '\n' && ch != '\0')
			continue;

		if (--line == 0) {
			prev = newline++;
			break;
		} else {
			newline = end + 1;
		}
	}

	if (line != 0)
		return NULL;

	len = end - prev;
	if (len == 0) /* \n\0 */
		return NULL;

	res = estrndup(&str[prev], len);
	return res;
}

/**
 * Returns the index of the last character in the given line.
 *
 * @param line Line for which last character should be returned.
 * @return Index of the last character in the given line.
 */
size_t
endofline(char *line)
{
	char *pos;

	if (!(pos = strchr(line, '\n')))
		pos = strchr(line, '\0');
	assert(pos != NULL);

	assert(pos - line >= 0);
	return (size_t)(pos - line);
}

/**
 * Compares the first 'n' bytes of two strings (like strncmp(3)).
 * However, unlike strncmp(3) it returns the position of the
 * first character that differed.
 *
 * @param s1 First string to use for comparison.
 * @param s2 Second string to use for comparison.
 * @param n Amount of bytes to compare.
 * @param res Pointer to an address where the position of
 * 	the first character that differed should be stored.
 * 	The first element is located at position 0.
 * @returns An integer less than, equal to, or greater than
 * 	zero if s1 (or the first n bytes thereof) is found,
 *	respectively, to be less than, to match, or be greater than s2.
 */
int
xstrncmp(char *s1, char *s2, size_t n, size_t *res)
{
	size_t i;

	i = 1;
	while (*s1 && *s1 == *s2 && i < n) {
		s1++;
		s2++;
		i++;
	}

	*res = --i;
	return *s1 - *s2;
}

/**
 * Reads the content of the given file at the given path and returns a
 * pointer to a newly allocated string containing the content.
 *
 * @param fp to file which should be read.
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
	size_t len, read;

	if (stat(fp, &st))
		return NULL;
	len = (size_t)st.st_size;

	fc = emalloc(len + 1);
	if (!(fd = fopen(fp, "r")))
		return NULL;

	read = fread(fc, sizeof(char), len, fd);
	if (ferror(fd))
		return NULL;
	if (fclose(fd))
		return NULL;

	fc[read] = '\0';
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
 * Calls realloc(3) but terminates the program with EXIT_FAILURE if realloc
 * returned an error.
 *
 * @param size Amount of memory (in bytes) which should be allocated.
 * @returns Pointer to the allocated memory.
 */
void*
erealloc(void *ptr, size_t size)
{
	void *r;

	if (!(r = realloc(ptr, size)))
		die("realloc failed");

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
