/* SccsId: @(#)node.h	1.8 1/2/84
 *
 * This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
 * Permission is granted to use and copy this software, but not for profit,
 * and provided that these same conditions are imposed on any person
 * receiving or using the software.
 */

/*
 * B editor -- Parse tree and Focus stack.
 */

/*
 * Assertion macro.
 *
 * This one differs from the one in #include <assert.h> in that it
 * is usable as an expression operand, e.g. up(ep) || Assert(No).
 * The function asserr() must unconditionally terminate the program.
 * If the accumulated __FILE__ data wastes too much of your data
 * space, omit them and change the code in asserr() that uses them.
 * You better trust your code then, because unless compiled with "-g"
 * it's difficult to dig the line number information from the core dump.
 *
 * There is also a variant called Abort() which is equivalent to Assert(No).
 */

#ifdef NDEBUG
#define Assert(cond) 0 /* Dummy expression */
#define Abort() abort() /* Always fail */
#else NDEBUG
#define Assert(cond) ((cond) || asserr(__FILE__, __LINE__))
#define Abort() asserr(__FILE__, __LINE__)
#endif NDEBUG

typedef struct node *node;
typedef struct path *path;
typedef int markbits;

struct node {
	char type;
	intlet len;
	intlet refcnt;
	int n_marks;
	intlet n_width;
	intlet n_symbol;
	node n_child[1];
};

struct path {
	char	type;
	intlet	len;
	intlet	refcnt;
	path	p_parent;
	node	p_tree;
	intlet	p_ichild;
	intlet	p_ycoord;
	intlet	p_xcoord;
	intlet	p_level;
	markbits	p_addmarks;
	markbits	p_delmarks;
};


#define Nnil ((node) NULL)

node newnode();

#ifndef NDEBUG
#define symbol(n) (Assert(Type(n)==Nod), (n)->n_symbol)
#define nchildren(n) (Assert(Type(n)==Nod), Length(n))
#define marks(n) (Assert(Type(n)==Nod), (n)->n_marks)
#define child(n, i) \
	(Assert(Type(n)==Nod && (i)>0 && (i)<=Length(n)), (n)->n_child[(i)-1])
#else NDEBUG
#define symbol(n) ((n)->n_symbol)
#define nchildren(n) (Length(n))
#define marks(n) ((n)->n_marks)
#define child(n, i) ((n)->n_child[(i)-1])
#endif NDEBUG

#define width(n) (Type(n)==Tex ? Length((value)(n)) : (n)->n_width)
#define marked(p, x) (marks(tree(p))&(x))

#define Pnil ((path) NULL)

path newpath();

#define parent(p) ((p)->p_parent)
#define tree(p) ((p)->p_tree)
#define ichild(p) ((p)->p_ichild)
#define getcoord(p, py, px, plevel) ((py) && (*(py)=(p)->p_ycoord), \
	(px) && (*(px)=(p)->p_xcoord), \
	(plevel) && (*(plevel)=(p)->p_level))
#define Ycoord(p) ((p)->p_ycoord)
#define Xcoord(p) ((p)->p_xcoord)
#define Level(p) ((p)->p_level)

/* Procedure markpath(); */
/* Procedure unmkpath(); */
/* Procedure replace(); */
bool up();
bool downi();

#define down(n) downi(n, 1)

bool downrite();
bool left();
bool rite();
/* Procedure top(); */
bool nextnode();
/* Procedure firstleaf(); */
bool nextleaf();
bool prevnode();
/* Procedure lastleaf(); */
bool prevleaf();
bool nextmarked();
bool prevmarked();

/*
 * The following are routines for lint, but macros for CC.
 * This way lint can detect wrong arguments passed.
 */

#ifdef lint

node nodecopy();
noderelease();
nodeuniql();

path pathcopy();
pathrelease();
pathuniql();

#else

#define nodecopy(n) ((node)copy(n))
#define noderelease(n) release(n)
#define nodeuniql(pn) uniql(pn)

#define pathcopy(p) ((path)copy(p))
#define pathrelease(p) release(p)
#define pathuniql(pp) uniql(pp)

#endif

node grab_node();
path grab_path();
