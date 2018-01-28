/* 
 * Siple Array list
 */

#ifndef _LIST
#define _LIST

struct element{
	char *name;
	void *value;
	
};

struct s_list{
	int length;
	struct element *elements;
};

struct s_list *list_init(void);
struct s_list *list_copy(struct s_list *list);

void list_free(struct s_list *list);
int list_length(struct s_list *list);
int list_find(struct s_list *list, char *name);
int list_add(struct s_list *list, char *name, void *value);
int list_remove(struct s_list *list, char *name);
char *list_getname(struct s_list *list, int index);
void *list_getvalue(struct s_list *list, int index);
void list_setvalue(struct s_list *list, int index, void *value);


/*Spezielle tools fuer den Comiler  */
void list_printdebug(struct s_list *list, const char *text);
struct s_list *list_tool_cple(struct s_list *list, char *ename, void *evalue, const char *error);
struct s_list *list_tool_cpll(struct s_list *l1, struct s_list *l2, const char *error);
struct s_list *list_tool_addle(struct s_list *list, char *ename, void *evalue, const char *error);
struct s_list *list_tool_addle_unsave(struct s_list *list, char *ename, void *evalue);
struct s_list *list_tool_addll(struct s_list *list, struct s_list *add, const char *error);
struct s_list *list_tool_addlll(struct s_list *list, struct s_list *add1, struct s_list *add2, const char *error);
struct s_list *list_tool_inite(char *name, char *value);

void list_tool_exclusive(struct s_list *l1, struct s_list *l2, char *error);
void list_tool_isinlist(struct s_list *list, char *name, char *error);



#endif /* _LIST */

