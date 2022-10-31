/* SccsId: @(#)queu.h	1.6 1/2/84
 *
 * This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
 * Permission is granted to use and copy this software, but not for profit,
 * and provided that these same conditions are imposed on any person
 * receiving or using the software.
 */

/*
 * B editor -- Definitions for queues of nodes.
 */

typedef struct queue *queue;

struct queue {
	char type;
	intlet len;
	intlet refcnt;
	node q_data;
	queue q_link;
};

#define Qnil ((queue) NULL)
#define qcopy(q) ((queue)copy((value)(q)))
#define qrelease(q) release((value)(q))
#define emptyqueue(q) (!(q))

node queuebehead();
