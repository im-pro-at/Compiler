/* 
 * Siple Array list Without a name
 */

#ifndef _VLIST_H_
#define _VLIST_H_

struct s_vlist{
	int length;
	void **values;
};

struct s_vlist *vlist_init(void);
void vlist_free(struct s_vlist *list);
int vlist_length(struct s_vlist *list);
int vlist_find(struct s_vlist *list, void *value);
int vlist_add(struct s_vlist *list, void *value);
int vlist_remove(struct s_vlist *list, void *value);
void *vlist_getvalue(struct s_vlist *list, int index);
void vlist_setvalue(struct s_vlist *list, int index, void *value);




#endif /* _VLIST_H_ */

