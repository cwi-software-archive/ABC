/* Private definitions for small texts, lists and tables module */

#define List_elem(l, i) (*(Ats(l)+i)) /*counts from 0; takes no copy*/

extern bool found();
extern value list_elem();
