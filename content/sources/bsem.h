/*
   This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
   Permission is granted to use and copy this software, but not for profit,
   and provided that these same conditions are imposed on any person
   receiving or using the software.
*/
/* bsem.h: semantics */

typedef bool outcome;
#define Succ Yes
#define Fail No
#define Und ((bool) '?') /* absence of REPORTed outcome */

#define Ifxeq(v) (xeq ? (v) : Dumval)

extern loc local_loc();
extern loc global_loc();

extern value content();
extern check_location();
extern loc trim_loc();
extern loc tbsel_loc();
extern put();
extern delete();
extern insert();
extern remove();
extern bind();

/* Functions and Predicates */
extern bool is_zerfun();
extern bool is_monfun();
extern bool is_dyafun();
extern bool is_zerprd();
extern bool is_monprd();
extern bool is_dyaprd();
extern value montor();
extern value dyator();
extern value formula();
extern outcome proposition();
extern initfprs();

/* Expressions: */
extern value expr();
extern value obasexpr();
extern value tag();
extern trimbc();
extern inittors();

/* Targets: */
extern loc targ();
extern loc bastarg();

/* Tests: */
extern outcome test();
extern bool relop();

/* Commands: */
extern command();
extern comm_suite();
extern initcom();

/* B units */

extern value resval; extern outcome resout;
extern bool terminated;

extern bool unit();
extern getunit();
extern ytu_heading();
extern bool udc();
extern udfpr();
extern bool ref_com();
extern ref_et();
extern value eva_formal();
extern loc loc_formal();
extern inithow();
