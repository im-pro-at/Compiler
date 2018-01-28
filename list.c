/* 
 * Siple Array List
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "list.h"


/* Initialisiert die Liste */
struct s_list *list_init(void){
	struct s_list *list;
	
	list=malloc(sizeof(struct s_list));
	if(list==NULL){
		printf("LIST: Memory Error!\n");
		exit(3);
	}	

	list->length=0;
	list->elements=NULL;
	return list;
}

void list_free(struct s_list *list){
	free(list->elements);
	free(list);
}

int list_length(struct s_list *list){
	return list->length;
}


/* Gibt die Position enes Elements Zurueck im Fehlerfall -1 */
int list_find(struct s_list *list, char *name){
	int i;

	if(list==NULL || name==NULL)
		return -1;

	for(i=0;i<list->length;i++){
		if(strcmp(name,list->elements[i].name)==0)
			return i;
	}
	return -1;	
}

/* Fuegt ein Element in die Liste ein und gitb die Position zurueck (-1 wenn name schon in Liste! -2 wenn name=null)*/
int list_add(struct s_list *list, char *name, void *value){
	if(list_find(list,name)!=-1)
		return -1;

	if(list==NULL || name==NULL)
		return -2;	

	//Array erweitern
	list->length++;
	list->elements=realloc(list->elements, list->length*sizeof(struct element));
	if(list->elements==NULL){
		printf("LIST: Memory Error!\n");
		exit(3);
	}	


	list->elements[list->length-1].name=name;
	list->elements[list->length-1].value=value;
	
	
	return list->length-1;
}

/* removes an Elements from List and returnes its old Position (-1 when name is not in list) */
int list_remove(struct s_list *list, char *name){
	int i;
	int epos=list_find(list,name);
	
	if(epos==-1)
		return -1;	

	for(i=epos; i<list->length-1;i++)
		list->elements[i]=list->elements[i+1];
	
	/* liste verkleinern */
	list->length--;
	list->elements=realloc(list->elements, list->length*sizeof(list->elements));
	if(list->length>0 && list->elements==NULL){
		printf("LIST: Memory Error!\n");
		exit(3);
	}
	
	return epos;
}

char *list_getname(struct s_list *list, int index){
	if(list==NULL || index<0 || index>=list->length)
		return "";
	return list->elements[index].name;	

}

/* Returns the Value of an element (NULL if Error) */
void *list_getvalue(struct s_list *list, int index){
	if(list==NULL || index<0 || index>=list->length)
		return NULL;
	return list->elements[index].value;
}

void list_setvalue(struct s_list *list, int index, void *value){
	if(list==NULL || index<0 || index>=list->length)
		return;
	list->elements[index].value=value;
}

void list_printdebug(struct s_list *list, const char *text){
	int i;
	D("%s [",text);
	if(list==NULL){
		D("!NULL!");
	}
	else{
		for(i=0;i<list->length;i++){
			D("%s ,",list->elements[i].name);
		}
	}
	D("]\n");
}

/*  Fuegt die Liste source zur lite target hinzu source bleibt unveraendert. 
    Gibt die Anzahl an hinzugefuegter Elemente zurueck -1 im Fehlerfall wenn -1 inhalt von target undefiniert 
*/
int list_addlist(struct s_list *target, struct s_list *source){
	int i;
	/* source darf nicht bearbeitet werden */
	if(target==NULL || target==source)
		return -1;
	
	if(source==NULL)
		return 0;

	for(i=0; i<source->length;i++)
		if(list_add(target,source->elements[i].name,source->elements[i].value)==-1)
			return -1;

	return source->length;
}

/* kopiert eine Liste */
struct s_list *list_copy(struct s_list *list){
	struct s_list *temp=list_init();
	list_addlist(temp, list);
	return temp;
}

/* erstellt eine neue Liste mit der alten und dem element */
struct s_list *list_tool_cple(struct s_list *list, char *ename, void *evalue, const char *error){
	struct s_list *new;
	new=list_init();
	
	list_addlist(new, list);
	if(list_add(new,ename,evalue)==-1){
		printf("OX: %s (%s)\n",error,ename);
		exit(3);
	}
	return new;
}

/* fuegt die Liste in die Liste ein und gibt die Liste zurueck */
struct s_list *list_tool_cpll(struct s_list *l1, struct s_list *l2, const char *error){
	if(l1==l2){
		printf("OX: list_tool_addll Internal error!");
		exit(3);
	}

	struct s_list *new= list_init();

	new=list_tool_addll(new,l1,error);		
	new=list_tool_addll(new,l2,error);		
	
	return new;
}


/* fuegt das element in die Liste ein und gibt die Liste zurueck */
struct s_list *list_tool_addle(struct s_list *list, char *ename, void *evalue, const char *error){

	if(list_add(list,ename,evalue)==-1){
		printf("OX: %s (%s)\n",error,ename);
		exit(3);
	}
	return list;
}

/* fuegt das element in die Liste ein und gibt die Liste zurueck */
struct s_list *list_tool_addle_unsave(struct s_list *list, char *ename, void *evalue){
	
	list_add(list,ename,evalue);
	
	return list;
}


/* fuegt die Liste in die Liste ein und gibt die Liste zurueck */
struct s_list *list_tool_addll(struct s_list *list, struct s_list *add, const char *error){
	int i;
	
	if(list==add){
		printf("OX: list_tool_addll Internal error!");
		exit(3);
	}
		
	
	if(add==NULL)
		return list;
	

	for(i=0; i<add->length;i++){
		if(list_add(list,add->elements[i].name,add->elements[i].value)==-1){
			printf("OX: %s (%s)\n",error,add->elements[i].name);
			exit(3);
		}
	}
	
	return list;

}

struct s_list *list_tool_addlll(struct s_list *list, struct s_list *add1, struct s_list *add2, const char *error){
	return list_tool_addll(list_tool_addll(list,add1,error),add2,error);
}

/* erstellt eine liste und fuegt das element hinzu */
struct s_list *list_tool_inite(char *name, char *value){
	struct s_list *new=list_init();

	list_add(new,name,value);
	
	return new;
}


/* schaut ob inhalte nicht dekcenet */
void list_tool_exclusive(struct s_list *l1, struct s_list *l2, char *error){
	int i;

	if (l1==NULL || l2==NULL)
		return;

	for(i=0;i<l2->length;i++){
		if(list_find(l1, l2->elements[i].name)!=-1){
			printf("OX: %s (%s)\n",error,l2->elements[i].name);
			exit(3);
		}
	}	

}

void list_tool_isinlist(struct s_list *list, char *name, char *error){
	if(list_find(list,name)==-1){
		printf("OX: %s (%s)\n",error,name);
		exit(3);	
	}	
}


