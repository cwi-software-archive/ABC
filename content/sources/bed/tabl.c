/* SccsId: @(#)tabl.c	1.11 1/4/84
 *
 * This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
 * Permission is granted to use and copy this software, but not for profit,
 * and provided that these same conditions are imposed on any person
 * receiving or using the software.
 */

/*
 * B editor -- Grammar table.
 */

#include "b.h"
#include "node.h"
#include "gram.h"
#include "tabl.h"


/*
 * ***** DISCLAIMER *****
 *
 * This file is a mess.  There should really be a separate program (like Yacc)
 * to compile a grammar into tables.  But for the time being . . .
 */


/*
 * Values returned by function symbol(n).
 * They are used directly as index in the grammar table.
 * The NAMES of the #defined constants are of no importance outside this file.
 */

#define Put	1
#define Insert	2
#define Remove	3
#define Choose	4
#define Draw	5
#define Set_random	6
#define Delete	7
#define Check	8
#define Share	9

#define Write	10
#define Read	11
#define Read_raw	12

#define If	13
#define While	14
#define For	15

#define Select	16
#define Test_suite	17

#define Quit	18
#define Return	19
#define Report	20
#define Succeed	21
#define Fail	22

#define How_to	23
#define Yield	24
#define Test	25
#define Suite	26
#define Refinement	27

#define Compound	28
#define Collateral	29
#define Tag	30
#define Number	31
#define Selection	32
#define Behead	33
#define Curtail	34

#define And	35
#define Or	36
#define Not	37
#define Some_in	38
#define Each_in	39
#define No_in	40
#define Some_parsing	41
#define Each_parsing	42
#define No_parsing	43

#define Comment	44
#define Keyword	45

#define L_t_dis	46
#define List_body	47
#define Tab_body	48
#define Tab_entry	49

#define E_number	50
#define Com_target	51
#define Col_target	52
#define Sel_expr	53
#define Text1	54
#define Text2	55
#define Grouped	56
#define Blocked	57
#define Operators	58

#define Else_kw	59
#define Kw_plus	60
#define E_plus	61
#define Conversion	62
#define T1	63
#define T1_plus	64
#define T2	65
#define T2_plus	66
#define Cmt_cmd	67

#define F_kw_plus	69
#define F_e_plus	70
#define Plus_sign	71
#define Minus_sign	72

#define Long_comp	73
#define Short_comp	74
#define Cmt_comp	75

#define Long_unit	76
#define Short_unit	77
#define Cmt_head	78

#define Ref_join	79

#define And_kw	80
#define Or_kw	81

/* The last two, `Optional' and `Hole', are defined in "gram.h". */


/*
 * Symbol values used for lexical elements.
 * Cross-reference: "lexi.c", table `chclass'.
 */

#define LEXICAL 100

#define IDENT (LEXICAL+0)
#define KEYWORD (LEXICAL+1)
#define NUMBER (LEXICAL+2)
#define COMMENT (LEXICAL+3)
#define TEXT1 (LEXICAL+4)
#define TEXT2 (LEXICAL+5)
#define OPERATORS (LEXICAL+6)


/*
 * Classes used in table initialization.
 */

#define TARGET Tag, Com_target, Selection, Behead, Curtail
#define PRIMARY Sel_expr, Tag, E_number, Number, \
	Compound, L_t_dis, Text1, Text2
#define EXPR Blocked, Grouped, Operators, PRIMARY

Hidden class Atag_body = {IDENT, 0};
	Hidden struct classinfo tag_body[] = {Atag_body};
Hidden class Anum_body = {NUMBER, 0};
	Hidden struct classinfo num_body[] = {Anum_body};
Hidden class Acom_body = {COMMENT, 0};
	Hidden struct classinfo com_body[] = {Acom_body};
Hidden class Akw_body = {KEYWORD, 0};
	Hidden struct classinfo kw_body[] = {Akw_body};
Hidden class At1_body = {TEXT1, 0};
	Hidden struct classinfo t1_body[] = {At1_body};
Hidden class At2_body = {TEXT2, 0};
	Hidden struct classinfo t2_body[] = {At2_body};
Hidden class Aops_body = {OPERATORS, 0};
	Hidden struct classinfo ops_body[] = {Aops_body};

Hidden class Aid_or_kw = {Tag, Keyword, 0};
	Hidden struct classinfo id_or_kw[] = {Aid_or_kw};
Hidden class Anumber = {Number, 0};
	Hidden struct classinfo number[] = {Anumber};
Hidden class Asign = {Optional, Plus_sign, Minus_sign, 0};
	Hidden struct classinfo sign[] = {Asign};

Hidden class Ao_c_expr = {Optional, Collateral, EXPR, 0};
	Hidden struct classinfo o_c_expr[] = {Ao_c_expr};

#define Ac_expr (Ao_c_expr+1)
	Hidden struct classinfo c_expr[] = {Ac_expr};
#define Aexpr (Ao_c_expr+2)
	Hidden struct classinfo expr[] = {Aexpr};
#define Aprimary (Ao_c_expr+5)
	Hidden struct classinfo primary[] = {Aprimary};

Hidden class Ablock = {Operators, PRIMARY, 0};
	Hidden struct classinfo block[] = {Ablock};
Hidden class Agroup = {Blocked, Operators, PRIMARY, 0};
	Hidden struct classinfo group[] = {Agroup};

#define Ar_expr Agroup
	Hidden struct classinfo r_expr[] = {Ar_expr};

Hidden class Al_t_body =	{Optional, List_body, PRIMARY, Blocked, 
	Grouped, Operators, Tab_body, Tab_entry, 0};
	Hidden struct classinfo l_t_body[] = {Al_t_body};
Hidden class Alist_body = {List_body, EXPR, 0};
	Hidden struct classinfo list_body[] = {Alist_body};
Hidden class Atab_body = {Tab_body, Tab_entry, 0};
	Hidden struct classinfo tab_body[] = {Atab_body};
Hidden class Atab_entry = {Tab_entry, 0};
	Hidden struct classinfo tab_entry[] = {Atab_entry};

Hidden class Ac_target = {Col_target, TARGET, 0};
	Hidden struct classinfo c_target[] = {Ac_target};

#define Atarget (Ac_target+1)
	Hidden struct classinfo target[] = {Atarget};

#define SOME_ETC Not, Some_in, Each_in, No_in, \
	Some_parsing, Each_parsing, No_parsing

Hidden class Ae_test = {Else_kw, SOME_ETC, And, Or, EXPR, 0};
	Hidden struct classinfo e_test[] = {Ae_test};

#define Atest (Ae_test+1)
	Hidden struct classinfo test[] = {Atest};
#define At_test Aexpr
	Hidden struct classinfo t_test[] = {At_test};
Hidden class Ar_test = {SOME_ETC, EXPR, 0};
	Hidden struct classinfo r_test[] = {Ar_test};
Hidden class Aand_test = {SOME_ETC, And, EXPR, 0};
	Hidden struct classinfo and_test[] = {Aand_test};
Hidden class Aor_test = {SOME_ETC, Or, EXPR, 0};
	Hidden struct classinfo or_test[] = {Aor_test};
Hidden class Ac_test = {Collateral, SOME_ETC, And, Or, EXPR, 0};
	Hidden struct classinfo c_test[] = {Ac_test};
	/*
	 * This means that a compound expression may in fact
	 * contain a `collateral test', e.g. (a AND b, c AND d).
	 * Of course, this is illegal in B, but I couldn't
	 * solve the ambiguity of `(' where a test is expected
	 * otherwise (this may start a parenthesized test, or
	 * a compound expression; the latter may be followed
	 * by more expression fragments, the first may not).
	 */

Hidden class Acomment = {Comment, 0};
	Hidden struct classinfo comment[] = {Acomment};
Hidden class Ao_comment = {Optional, Comment, 0};
	Hidden struct classinfo o_comment[] = {Ao_comment};
	
#define HEAD How_to, Yield, Test
#define BODY HEAD, Cmt_head, Long_unit, Short_unit

Hidden class Ac_head = {Cmt_head, HEAD, 0};
	Hidden struct classinfo c_head[] = {Ac_head};
#define Ahead (Ac_head+1)
	Hidden struct classinfo head[] = {Ahead};

Hidden class Aunit = {Optional, EXPR, BODY, Ref_join, 0};
	Hidden struct classinfo unit[] = {Aunit};
Hidden class Ao_refinements = {Optional, Refinement, 0};
	Hidden struct classinfo o_refinements[] = {Ao_refinements};
#define Arefinements (Ao_refinements+1)
	Hidden struct classinfo refinements[] = {Arefinements};
Hidden class Arefpred = {BODY, 0};
	Hidden struct classinfo refpred[] = {Arefpred};

Hidden class Af_cmd = {Keyword, F_kw_plus, 0};
	Hidden struct classinfo f_cmd[] = {Af_cmd};
#define Af_formula Aexpr /*****/
	Hidden struct classinfo f_formula[] = {Af_formula};

Hidden class Ao_suite = {Optional, Suite, 0};
	Hidden struct classinfo o_suite[] = {Ao_suite};
Hidden class At_suite = {Test_suite, 0};
	Hidden struct classinfo t_suite[] = {At_suite};
Hidden class Ao_t_suite = {Optional, Test_suite, 0};
	Hidden struct classinfo o_t_suite[] = {Ao_t_suite};

/* The order here determines which are suggested first and is subject
   to constant change! */
#define SIMPLE_CMD \
			Quit, Return, \
			Write, Read, Read_raw, \
			Put, Delete, Insert, Remove, \
			Report, Fail, Succeed, \
			Check, \
			Choose, Draw, Set_random, \
			Share, \
			Keyword, Kw_plus
#define CONTROL_CMD If, While, For
#define COMP_CMD Short_comp, Long_comp, Cmt_comp, Select
#define CMD CONTROL_CMD, COMP_CMD, SIMPLE_CMD
#define SHORTCMD CONTROL_CMD, SIMPLE_CMD, Short_comp, Cmt_comp, Cmt_cmd

Hidden class Acmd = {Comment, CMD, Cmt_cmd, 0};
	Hidden struct classinfo cmd[] = {Acmd};
Hidden class Ashortcmd = {SHORTCMD, 0};
	Hidden struct classinfo shortcmd[] = {Ashortcmd};
Hidden class Ao_cmdsuite = {Optional, SHORTCMD, Suite, 0};
	Hidden struct classinfo o_cmdsuite[] = {Ao_cmdsuite};
Hidden class Asuite = {Suite, 0};
	Hidden struct classinfo suite[] = {Asuite};
Hidden class Asimple_cmd = {SIMPLE_CMD, 0};
	Hidden struct classinfo simple_cmd[] = {Asimple_cmd};

Hidden class Ac_ifforwhile = {CONTROL_CMD, Cmt_comp, 0};
	Hidden struct classinfo c_ifforwhile[] = {Ac_ifforwhile};
Hidden class Aifforwhile = {CONTROL_CMD, 0};
	Hidden struct classinfo ifforwhile[] = {Aifforwhile};

Hidden class Akeyword = {Keyword, 0};
	Hidden struct classinfo keyword[] = {Akeyword};
Hidden class Akw_next = {Collateral, EXPR, Keyword, Kw_plus, E_plus, 0};
	Hidden struct classinfo kw_next[] = {Akw_next};
Hidden class Ae_next = {Keyword, Kw_plus, 0};
	Hidden struct classinfo e_next[] = {Ae_next};

Hidden class Af_kw_next = {Tag, Keyword, F_kw_plus, F_e_plus, 0};
	Hidden struct classinfo f_kw_next[] = {Af_kw_next};
Hidden class Af_e_next = {Keyword, F_kw_plus, 0};
	Hidden struct classinfo f_e_next[] = {Af_e_next};
Hidden class Atag = {Tag, 0};
	Hidden struct classinfo tag[] = {Atag};

Hidden class Atext1 = {Optional, T1, Conversion, T1_plus, 0};
	Hidden struct classinfo text1[] = {Atext1};
Hidden class At1_conv = {T1, Conversion, 0};
	Hidden struct classinfo t1_conv[] = {At1_conv};
Hidden class At1_next = {T1, Conversion, T1_plus, 0};
	Hidden struct classinfo t1_next[] = {At1_next};

Hidden class Atext2 = {Optional, T2, Conversion, T2_plus, 0};
	Hidden struct classinfo text2[] = {Atext2};
Hidden class At2_conv = {T2, Conversion, 0};
	Hidden struct classinfo t2_conv[] = {At2_conv};
Hidden class At2_next = {T2, Conversion, T2_plus, 0};
	Hidden struct classinfo t2_next[] = {At2_next};

Hidden class Aand = {And_kw, 0};
	Hidden struct classinfo and[] = {Aand};
Hidden class Aor = {Or_kw, 0};
	Hidden struct classinfo or[] = {Aor};


/*
 * WARNING: The entries in this table must correspond one by one
 * to the symbols defined earlier.  This is checked dynamically
 * by the initialization procedure (syserr "table order").
 */

#define XX(name) name, "name"

Hidden struct table b_grammar[] = {
	{XX(Rootsymbol), {0}, {unit}}, /* Start symbol of the grammar */
	{XX(Put), {"PUT ", " IN "}, {c_expr, c_target}},
	{XX(Insert), {"INSERT ", " IN "}, {c_expr, target}},
	{XX(Remove), {"REMOVE ", " FROM "}, {c_expr, target}},
	{XX(Choose), {"CHOOSE ", " FROM "}, {c_expr, expr}},
	{XX(Draw), {"DRAW "}, {target}},
	{XX(Set_random), {"SET'RANDOM "}, {c_expr}},
	{XX(Delete), {"DELETE "}, {c_target}},
	{XX(Check), {"CHECK "}, {test}},
	{XX(Share), {"SHARE "}, {c_target}},

	{XX(Write), {"WRITE "}, {c_expr}},
	{XX(Read), {"READ ", " EG "}, {c_target, c_expr}},
	{XX(Read_raw), {"READ ", " RAW"}, {target}},

	{XX(If), {"IF ", ": "}, {test}},
	{XX(While), {"WHILE ", ": "}, {test}},
	{XX(For), {"FOR ", " IN ", ": "}, {c_target, expr}},

	{XX(Select), {"SELECT: ", "\t", "\b"}, {o_comment, t_suite}},
	{XX(Test_suite), {"\n", ": ", "\t", "\b"},
		{e_test, o_comment, o_cmdsuite, o_t_suite}},

	{XX(Quit), {"QUIT"}, {0}},
	{XX(Return), {"RETURN "}, {c_expr}},
	{XX(Report), {"REPORT "}, {test}},
	{XX(Succeed), {"SUCCEED"}, {0}},
	{XX(Fail), {"FAIL"}, {0}},

	{XX(How_to), {"HOW'TO ", ": "}, {f_cmd}},
	{XX(Yield), {"YIELD ", ": "}, {f_formula}},
	{XX(Test), {"TEST ", ": "}, {f_formula}},

	{XX(Suite), {"\n"}, {cmd, o_suite}},
	{XX(Refinement), {"\n", ": ", "\t", "\b"},
		{id_or_kw, o_comment, o_cmdsuite, o_refinements}},

	{XX(Compound), {"(", ")"}, {c_test}},
	{XX(Collateral), {0, ", "}, {expr, c_expr}},
	{XX(Tag), {0}, {tag_body}},
	{XX(Number), {0}, {num_body}},
	{XX(Selection), {0, "[", "]"}, {target, c_expr}},
	{XX(Behead), {0, "@"}, {target, r_expr}},
	{XX(Curtail), {0, "|"}, {target, r_expr}},

	{XX(And), {0, " "}, {t_test, and}},
	{XX(Or), {0, " "}, {t_test, or}},
	{XX(Not), {"NOT "}, {r_test}},
	{XX(Some_in), {"SOME ", " IN ", " HAS "}, {c_target, expr, r_test}},
	{XX(Each_in), {"EACH ", " IN ", " HAS "}, {c_target, expr, r_test}},
	{XX(No_in), {"NO ", " IN ", " HAS "}, {c_target, expr, r_test}},
	{XX(Some_parsing), {"SOME ", " PARSING ", " HAS "},
		{c_target, expr, r_test}},
	{XX(Each_parsing), {"EACH ", " PARSING ", " HAS "},
		{c_target, expr, r_test}},
	{XX(No_parsing), {"NO ", " PARSING ", " HAS "}, {c_target, expr, r_test}},

	{XX(Comment), {0}, {com_body}},
	{XX(Keyword), {0}, {kw_body}},

	{XX(L_t_dis), {"{", "}"}, {l_t_body}},
	{XX(List_body), {0, "; "}, {expr, list_body}},
	{XX(Tab_body), {0, "; "}, {tab_entry, tab_body}},
	{XX(Tab_entry), {"[", "]: "}, {c_expr, expr}},
	{XX(E_number), {0, "E"}, {number, sign, number}},

	{XX(Com_target), {"(", ")"}, {c_target}},
	{XX(Col_target), {0, ", "}, {target, c_target}},
	{XX(Sel_expr), {0, "[", "]"}, {primary, c_expr}},

	{XX(Text1), {"'", "'"}, {text1}},
	{XX(Text2), {"\"", "\""}, {text2}},
	{XX(Grouped), {0, " "}, {group, expr}},
	{XX(Blocked), {0}, {block, group}},
	{XX(Operators), {0}, {ops_body}},
	{XX(Else_kw), {"ELSE"}, {0}},
	{XX(Kw_plus), {0, " "}, {keyword, kw_next}},
	{XX(E_plus), {0, " "}, {c_expr, e_next}},
	{XX(Conversion), {"`", "`"}, {o_c_expr}},
	{XX(T1), {0}, {t1_body}},
	{XX(T1_plus), {0}, {t1_conv, t1_next}},
	{XX(T2), {0}, {t2_body}},
	{XX(T2_plus), {0}, {t2_conv, t2_next}},
	{XX(Cmt_cmd), {0, " "}, {simple_cmd, comment}},
	{0},
	{XX(F_kw_plus), {0, " "}, {keyword, f_kw_next}},
	{XX(F_e_plus), {0, " "}, {tag, f_e_next}},
	{XX(Plus_sign), {"+"}, {0}},
	{XX(Minus_sign), {"-"}, {0}},

	{XX(Long_comp), {0, "\t", "\b"}, {c_ifforwhile, suite}},
	{XX(Short_comp), {0, "\t", "\b"}, {ifforwhile, shortcmd}},
	{XX(Cmt_comp), {0}, {ifforwhile, comment}},

	{XX(Long_unit), {0, "\t", "\b"}, {c_head, suite}},
	{XX(Short_unit), {0, "\t", "\b"}, {head, shortcmd}},
	{XX(Cmt_head), {0}, {head, comment}},

	{XX(Ref_join), {0}, {refpred, refinements}},

	{XX(And_kw), {"AND "}, {and_test}},
	{XX(Or_kw), {"OR "}, {or_test}},

	/* Spare(s); change Optional and Hole in "gram.h" if you run out. */

	{0}, {0}, {0}, {0}, {0},
	{0}, {0}, {0}, {0}, {0},
	{0}, {0}, {0}, {0}, {0},
	{0},

	/* Next two entries must be the last entries of the table. */
	/* (See comments in "gram.c", initgram().) */

	{XX(Optional), {0}, {0}},
	{XX(Hole), {"?"}, {0}},
};

Visible struct table *table = b_grammar;


/*
 * Table indicating which symbols are used to form lists of items.
 * Consulted via predicate 'issublist' in "gram.c".
 */

Hidden class Asublists = {
	E_plus, F_e_plus, 
	And, And_kw, Or, Or_kw,
	0,
};

Hidden struct classinfo sublists[] = {Asublists};


/*
 * Predicate telling whether two symbols can form lists together.
 * This is important for list whose elements must alternate in some
 * way, as is the case for [KEYWORD [expression] ]*.
 *
 * This code must be in this file, otherwise the names and values
 * of the symbols would have to be made public.
 */

Visible bool
samelevel(sym, sym1)
	register int sym;
	register int sym1;
{
	register int zzz;

	if (sym1 == sym)
		return Yes;
	if (sym1 < sym)
		zzz = sym, sym = sym1, sym1 = zzz; /* Ensure sym <= sym1 */
	/* Now always sym < sym1 */
	return sym == Kw_plus && sym1 == E_plus
		|| sym == F_kw_plus && sym1 == F_e_plus
		|| sym == And && sym1 == And_kw
		|| sym == Or && sym1 == Or_kw;
}


/*
 * Predicate to tell whether a symbol can form chained lists.
 * By definition, all right-recursive symbols can do so;
 * in addition, those listed in the class 'sublists' can do
 * it, too (this is used for lists formed of alternating members
 * such as KW expr KW ...).
 */

Visible bool
issublist(sym)
	register int sym;
{
	register int i;
	register string repr;

	Assert(sym < TABLEN);
	repr = table[sym].r_repr[0];
	if (Fw_positive(repr))
		return No;
	for (i = 0; i < MAXCHILD && table[sym].r_class[i]; ++i)
		;
	if (i <= 0)
		return No;
	repr = table[sym].r_repr[i];
	if (!Fw_zero(repr))
		return No;
	return isinclass(sym, table[sym].r_class[i-1])
		|| isinclass(sym, sublists);
}
