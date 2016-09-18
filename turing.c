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
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>

#include "util.h"
#include "turing.h"

/**
 * Macro used to iterate over all entries of a tmmap.
 *
 * @param MAP Pointer to a tmmap to iterate over.
 * @param VARNAME Variable name used for current item.
 */
#define MAP_FOREACH(MAP, VARNAME) \
	for (int i = 0; i < MAP->size; i++) \
		for (VARNAME = MAP->entries[i]; \
			VARNAME; VARNAME = VARNAME->next) \

/**
 * Allocates memory for a tmmap and initializes it.
 *
 * @param size Amount of buckets that should be used.
 * @returns Pointer to the initialized tmmap.
 */
tmmap*
newtmmap(int size)
{
	tmmap *map;

	map = emalloc(sizeof(tmmap));
	map->size = size;
	map->entries = emalloc(sizeof(mapentry) * size);

	for (int i = 0; i < size; i++)
		map->entries[i] = NULL;

	return map;
}

/**
 * Hash function for bucket hashing.
 *
 * @param map Map to calculate hash for.
 * @param key Key that should be hashed.
 * @returns Hash of the given key.
 */
int
hash(tmmap *map, int key)
{
	/* int value; */
	/* unsigned long hash = 5381; */

	/* value = key; */
	/* for (int i = 1; value; i *= 10) { */
	/* 	value = (value / i) % 10; */
	/* 	hash = ((hash << 5) + hash) + value; */
	/* } */

	return key % map->size;
}

/**
 * Allocates memory and initializes a new map entry / bucket for the tmmap.
 *
 * @param key Key which should be used for this entry.
 * @returns Pointer to initialized mapentry.
 */
mapentry*
newmapentry(int key)
{
	mapentry *ent;

	ent = emalloc(sizeof(mapentry));
	ent->key = key;
	ent->next = NULL;
	return ent;
}

/**
 * Frees allocated memory for a map entry.
 *
 * @param ent Pointer to map entry which should be freed.
 */
void
freemapentry(mapentry *ent)
{
	mapentry *next;

	assert(ent);
	for (next = ent->next; next != NULL; next = next->next)
		freemapentry(next);

	free(ent);
}

/**
 * Adds a new value to the tmmap, if the key is not already present.
 *
 * @param map Map to which a key should be added.
 * @param ent Entry which should be added.
 * @returns -1 if the key was already present, 0 otherwise.
 */
int
setval(tmmap *map, mapentry *ent)
{
	int mkey;
	mapentry *buck, *last, *next;

	mkey = hash(map, ent->key);
	if (!(buck = map->entries[mkey])) {
		map->entries[mkey] = ent;
		return 0;
	}

	for (next = buck; next != NULL; next = next->next) {
		if (ent->key == next->key)
			return -1;

		last = next;
	}

	last->next = ent;
	return 0;
}

/**
 * Reads the associated value for a given key from the map.
 *
 * @param map Map to use for key lookup.
 * @param key Key which should be looked up.
 * @param dest Pointer to mapentry used to store the associated value.
 * @returns -1 if a value for the given key didn't exist, 0 otherwise.
 */
int
getval(tmmap *map, int key, mapentry *dest)
{
	int mkey;
	mapentry *buck, *next;

	mkey = hash(map, key);
	if (!(buck = map->entries[mkey]))
		return -1;

	for (next = buck; next != NULL && next->key != key; next = next->next)
		;

	if (next == NULL)
		return -1;

	*dest = *next;
	return 0;
}

/**
 * Allocates memory for a new tape entry and initializes it.
 *
 * @param value Symbol of this tape entry.
 * @param prev Entry on the left hand side of this one.
 * @param next Entry on the right hand side of this one.
 * @returns Pointer to the newly created tape entry.
 */
tapeentry*
newtapeentry(char value, tapeentry *prev, tapeentry *next)
{
	tapeentry *entr;

	entr = emalloc(sizeof(tapeentry));
	entr->value = value;
	entr->next  = next;
	entr->prev  = prev;
	return entr;
}

/**
 * Allocates memory for a new state and initializes it.
 *
 * @returns Pointer to the newly created state.
 */
tmstate*
newtmstate(void)
{
	tmstate *state;

	state = emalloc(sizeof(tmstate));
	state->name = -1;
	state->trans = newtmmap(TRANSMAPSIZ);
	return state;
}

/**
 * Allocates memory for a new turing maschine and initializes it.
 *
 * @returns Pointer to the newly created turing maschine.
 */
dtm*
newtm(void)
{
	dtm *tm;

	tm = emalloc(sizeof(dtm));
	tm->states = newtmmap(STATEMAPSIZ);
	tm->start = tm->acceptsiz = 0;
	tm->tape = tm->first = newtapeentry(BLANKCHAR, NULL, NULL);
	return tm;
}

/**
 * Adds a new state to an exsting turing maschine.
 *
 * @param tm Turing maschine to which a state should be added.
 * @param state State which should be added to the turing maschine.
 * @retruns -1 if a state with the given name already exists, 0 otherwise.
 */
int
addstate(dtm *tm, tmstate *state)
{
	int ret;
	mapentry *entry;

	entry = newmapentry(state->name);
	entry->data.state = state;

	if ((ret = setval(tm->states, entry)))
		freemapentry(entry);

	return ret;
}

/**
 * Retrieves a state form an exsting turing maschine.
 *
 * @param tm Turing machine from which a state should be retrieved.
 * @param name Name of the state that should be retrieved.
 * @param dest Pointer to a state which should be used for storing the result.
 * @returns -1 if the state doesn't exist, 0 otherwise.
 */
int
getstate(dtm *tm, int name, tmstate *dest)
{
	int ret;
	mapentry entry;

	if ((ret = getval(tm->states, name, &entry)))
		return ret;

	*dest = *entry.data.state;
	return ret;
}

/**
 * Adds a transition to an existing turing maschine state.
 *
 * @param state State to which a new transition should be added.
 * @param trans Pointer to the transition which should be added to the state.
 * @returns -1 if a state with the given symbol already exists, 0 otherwise.
 */
int
addtrans(tmstate *state, tmtrans *trans)
{
	int ret;
	mapentry *entry;

	entry = newmapentry(trans->rsym);
	entry->data.trans = trans;

	if ((ret = setval(state->trans, entry)))
		freemapentry(entry);

	return ret;
}

/**
 * Retrieves a transition from an existing turing state.
 *
 * @param state State from which a transition should be extracted.
 * @param rsym Symbol which triggers the tranisition.
 * @param dest Pointer to a transition which should be used for storing the result.
 * @returns -1 if a tranisition with the given symbol doesn't exist, 0 otherwise.
 */
int
gettrans(tmstate *state, int rsym, tmtrans *dest)
{
	int ret;
	mapentry entry;

	if ((ret = getval(state->trans, rsym, &entry)))
		return ret;

	*dest = *entry.data.trans;
	return ret;
}

/**
 * Writes the given string to the tape of the given turing maschine.
 *
 * @param tm Turing machine to modify tape of.
 * @param str String which should be written to the tape.
 */
void
writetape(dtm *tm, char *str)
{
	tapeentry *ent, *last;
	char c;

	for (tapeentry *i = tm->tape; i; i = i->next)
		last = i;

	while ((c = *str++)) {
		ent = newtapeentry(c, last, NULL);
		last->next = ent;
		last = ent;
	}
}

/**
 * Reads the string currently stored on the tape of the given turing maschine.
 * The string starts at the beginnig of the tape and ends as soons as the first
 * blank character is encountered.
 *
 * @param tm Turing machine whose tape should be read.
 * @param dest Pointer to a buffer where the result should be stored.
 * 	The result will be terminated by a null byte ('\0'). If the tape is
 * 	currently empty the only thing stored will be the null byte.
 */
void
readtape(dtm *tm, char *dest)
{
	int i;
	tapeentry *c;

	for (i = 0, c = tm->first->next; c &&
			c->value != BLANKCHAR; i++, c = c->next)
		dest[i] = c->value;
	dest[i] = '\0';
}

/**
 * Whether or not the given state name maps to an accepting state.
 *
 * @param tm Turing machine which defines the accepting states.
 * @param name State name to check.
 * @returns 0 if it does, -1 if it doesn't.
 */
int
isaccepting(dtm *tm, int name)
{
	for (int i = 0; i < tm->acceptsiz; i++)
		if (tm->accept[i] == name)
			return 0;

	return -1;
}

/**
 * Performs transitions form the given state tail recursively until a state
 * without any new transitions for the current tape symbol is reached.
 *
 * @param tm Turing machine to perform transitions on.
 * @param state State to perform transitions from.
 * @return 0 if the reached state is an accepting state, -1 otherwise.
 */
int
compute(dtm *tm, tmstate *state)
{
	char in;
	tmtrans trans;
	tmstate next;

	if (!tm->tape->next)
		tm->tape->next = newtapeentry(BLANKCHAR, tm->tape, NULL);

	in = tm->tape->next->value;
	if (gettrans(state, in, &trans))
		return isaccepting(tm, state->name);

	tm->tape->next->value = trans.wsym;
	switch (trans.headdir) {
	case RIGHT:
		tm->tape = tm->tape->next;
		break;
	case LEFT:
		if (!tm->tape->prev)
			tm->tape->prev = newtapeentry(BLANKCHAR,
				NULL, tm->tape);
		tm->tape = tm->tape->prev;
		break;
	case STAY:
		/* Nothing to do here. */
		break;
	}

	if (getstate(tm, trans.nextstate, &next))
		return isaccepting(tm, trans.nextstate);

	return compute(tm, &next);
}

/**
 * Starts the turing machine. Meaning it will extract the initial state from
 * the given tm and will perform transitions (tail recursively) from this state
 * until a state without any further transitions is reached.
 *
 * @param tm Turing machine which should be started.
 * @return 0 if the reached state is an accepting state, -1 otherwise.
 */
int
runtm(dtm *tm)
{
	tmstate start;

	if (getstate(tm, tm->start, &start))
		return isaccepting(tm, tm->start);

	return compute(tm, &start);
}

/**
 * Iterates over each state of the given turing machine and
 * invokes the given function for that state.
 *
 * @param tm Turing machine to iterate over.
 * @param fn Function to invoke for each state.
 * @param arg Additional argument to passed to the function.
 */
void
eachstate(dtm *tm, void (*fn)(tmstate*, void*), void *arg)
{
	tmmap *map;
	mapentry *elem;

	map = tm->states;
	MAP_FOREACH(map, elem)
		(*fn)(elem->data.state, arg);
}

/**
 * Iterates over each transition of the given state and invokes
 * the given function for that transition
 *
 * @param state Turing state to iterate over.
 * @param fn Function to invoke for each state.
 * @param arg Additional argument passed to the function.
 */
void
eachtrans(tmstate *state, void (*fn)(tmtrans*, tmstate*, void*), void *arg)
{
	tmmap *map;
	mapentry *elem;

	map = state->trans;
	MAP_FOREACH(map, elem)
		(*fn)(elem->data.trans, state, arg);
}

/**
 * Returns a char representation for a head direction.
 *
 * @param direction Head direction which should be converted.
 * @return Char describing the direction.
 */
char
dirstr(direction dir)
{
	switch (dir) {
	case RIGHT:
		return 'r';
	case LEFT:
		return 'l';
	case STAY:
		return 'n';
	}

	/* Never reached. */
	return -1;
}
