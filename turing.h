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

#ifndef _TMSIM_TURING_H
#define _TMSIM_TURING_H

#include <sys/types.h>

enum {
	/**
	 * Amount of buckets used for the state map.
	 */
	STATEMAPSIZ = 128,

	/**
	 * Amount of buckets used for the transition map.
	 */
	TRANSMAPSIZ = 16,

	/**
	 * Amount of space allocated for accepting states with realloc.
	 */
	ACCEPTSTEP = 8,

	/**
	 * Character used to represent blanks on the tape.
	 */
	BLANKCHAR = '$',
};

/**
 * Type used for turing machine state names.
 */
typedef unsigned int tmname;

/**
 * Enum describing direction head should be moved in.
 */
typedef enum {
	RIGHT,	/**< Move head right. */
	LEFT,	/**< Move head left. */
	STAY,	/**< Don't move head at all. */
} direction;

typedef struct _tmstate tmstate; /**< State of a turing machine. */
typedef struct _tmtrans tmtrans; /**< Transition from one state to another. */

/**
 * Type used as a key for the ::tmmap.
 */
typedef unsigned int mapkey;

/**
 * Entry in a bucket hashing implementation.
 */
typedef struct _mapentry mapentry;

struct _mapentry {
	mapkey key;	/**< Key of this entry. */

	/**
	 * If there was a hash collision bucket hashing is used which means
	 * we will use a linked list for this key. This element points to the
	 * next element in the linked list. If there wasn't a hash collision for
	 * the given key so far this field has the value NULL.
	 */
	mapentry *next;

	union {
		tmstate *state; /**< Used if this entry stores a tmstate. */
		tmtrans *trans; /**< Used if this entry stores a tmtrans. */
	} data;
};

/**
 * Bucket hashing implementation.
 */
typedef struct _tmmap tmmap;

struct _tmmap {
	size_t size;		/**< Amount of buckets which should be used. */
	mapentry **entries;	/**< Pointer pointing to entry pointers. */
};

struct _tmstate {
	tmname name;	/**< Name of this tmstate. */
	tmmap *trans;	/**< Transitions for this state. */
};

struct _tmtrans {
	/**
	 * Symbol which needs to be read to trigger this transition.
	 */
	unsigned char rsym;

	/**
	 * Symbol which should be written on the tape when performing this
	 * tranisition. The symbol which triggers this transition is not
	 * stored in this struct and is used as a key in the tmmap instead.
	 */
	unsigned char wsym;

	/**
	 * Direction to move head to after performing this transition.
	 */
	direction headdir;

	/**
	 * Name of the state the turing machine switches to after writing
	 * the associated symbol to the tape and moving the head to the
	 * associated direction.
	 */
	tmname nextstate;
};

/**
 * Double linked list used for entries on the tape of the turing machine.
 */
typedef struct _tapeentry tapeentry;

struct _tapeentry {
	unsigned char value;	/**< Symbol for this tape entry. */
	tapeentry *next;	/**< Entry on the right-hand side of this one. */
	tapeentry *prev;	/**< Entry on the left-hand side of this one. */
};

/**
 * Deterministic turing machine implementation.
 */
typedef struct _dtm dtm;

struct _dtm {
	tapeentry *tape;	/**< Tape content. */
	tmmap *states;		/**< Map of all states. */
	tmname start;		/**< Initial state. */

	tmname *accept;		/**< Pointer to array of accepting states. */
	size_t acceptsiz;	/**< Amount of accepting states. */
};

dtm *newtm(void);
tmstate *newtmstate(void);
void addaccept(dtm*, tmname);

int addtrans(tmstate*, tmtrans*);
int gettrans(tmstate*, unsigned char, tmtrans**);

int addstate(dtm*, tmstate*);
int getstate(dtm*, tmname, tmstate**);

void writetape(dtm*, char*);
void printtape(dtm*);

void eachstate(dtm*, void (*fn)(tmstate*, void*), void*);
void eachtrans(tmstate*, void(*fn)(tmtrans*, tmstate*, void*), void*);

int runtm(dtm*);
char dirstr(direction);
int verifyinput(char*, size_t*);

#endif
