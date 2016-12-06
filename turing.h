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

enum {
	STATEMAPSIZ = 512,	/**< Amount of buckets in the state map. */
	TRANSMAPSIZ = 256,	/**< Amount of buckets in the transition map. */
	MAXACCEPT = 512,	/**< Max amount of items in the accept state array. */
	BLANKCHAR = '$',	/**< Character used to represent blanks on the tape. */
};

/**
 * Enum discribing direction head should be moved in.
 */
typedef enum {
	RIGHT,	/**< Move head right. */
	LEFT,	/**< Move head left. */
	STAY,	/**< Don't move head at all. */
} direction;

typedef struct _tmstate tmstate; /**< State of a turing maschine. */
typedef struct _tmtrans tmtrans; /**< Transition from one state to another. */

/**
 * Entry in a bucket hashing implementation.
 */
typedef struct _mapentry mapentry;

struct _mapentry {
	int key;	/**< Key of this entry. */

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
	int name;	/**< Name of this tmstate. */
	tmmap *trans;	/**< Transitions for this state. */
};

struct _tmtrans {
	/**
	 * Symbol which needs to be read to trigger this transition.
	 */
	char rsym;

	/**
	 * Symbol which should be written on the tape when performing this
	 * tranisition. The symbol which triggers this transition is not
	 * stored in this struct and is used as a key in the tmmap instead.
	 */
	char wsym;

	/**
	 * Direction to move head to after performing this transition.
	 */
	direction headdir;

	/**
	 * Name of the state the turing maschine switches to after writing
	 * the associated symbol to the tape and moving the head to the
	 * associated direction.
	 */
	int nextstate;
};

/**
 * Double linked list used for entries on the tape of the turing maschine.
 */
typedef struct _tapeentry tapeentry;

struct _tapeentry {
	char value;		/**< Symbol for this tape entry. */
	tapeentry *next;	/**< Entry on the right-hand side of this one. */
	tapeentry *prev;	/**< Entry on the left-hand side of this one. */
};

/**
 * Deterministic turing maschine implementation.
 */
typedef struct _dtm dtm;

struct _dtm {
	tapeentry *tape;	/**< Current tape content of this turing maschine. */
	tapeentry *first;	/**< Pointer to the anchor element of the tape. */

	tmmap *states;		/**< States of this turing maschine. */
	int start;		/**< Initial state for this turing maschine. */

	size_t acceptsiz;	/**< Amount of accepting state of this turing maschine. */
	int accept[MAXACCEPT];	/**< Accepting states of this turing maschine. */
};

dtm *newtm(void);
tmstate *newtmstate(void);

int addtrans(tmstate*, tmtrans*);
int gettrans(tmstate*, int, tmtrans*);

int addstate(dtm*, tmstate*);
int getstate(dtm*, int, tmstate*);

void writetape(dtm*, char*);
size_t readtape(dtm*, char*, size_t);

void eachstate(dtm*, void (*fn)(tmstate*, void*), void*);
void eachtrans(tmstate*, void(*fn)(tmtrans*, tmstate*, void*), void*);

int runtm(dtm*);
char dirstr(direction);
int verifyinput(char*, size_t*);
