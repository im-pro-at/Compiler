/* 
 * Siple Array List without a name
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "vlist.h"


/* Initialisiert die Liste */
struct s_vlist *vlist_init(void){
	struct s_vlist *list;
	
	list=malloc(sizeof(struct s_vlist));
	if(list==NULL){
		printf("VLIST: Memory Error!\n");
		exit(3);
	}	

	list->length=0;
	list->values=NULL;
	return list;
}

void vlist_free(struct s_vlist *list){
	free(list->values);
	free(list);
}

int vlist_length(struct s_vlist *list){
	return list->length;
}


int vlist_find(struct s_vlist *list, void *value){
	int i;

	if(list==NULL || value==NULL)
		return -1;

	for(i=0;i<list->length;i++){
		if(list->values[i]==value)
			return i;
	}
	return -1;	
}

/* Fuegt ein Element in die Liste ein und gitb die Position zurueck (-1 wenn name schon in Liste! -2 wenn name=null)*/
int vlist_add(struct s_vlist *list, void *value){
	if(vlist_find(list,value)!=-1)
		return -1;

	if(list==NULL || value==NULL)
		return -2;	

	//Array erweitern
	list->length++;
	list->values=realloc(list->values, list->length*sizeof(void *));
	if(list->values==NULL){
		printf("VLIST: Memory Error!\n");
		exit(3);
	}	


	list->values[list->length-1]=value;
	
	return list->length-1;
}


/* removes an Elements from List and returnes its old Position (-1 when name is not in list) */
int vlist_remove(struct s_vlist *list, void *value){
	int i;
	int epos=vlist_find(list,value);
	
	if(epos==-1)
		return -1;	

	for(i=epos; i<list->length-1;i++)
		list->values[i]=list->values[i+1];
	
	/* liste verkleinern */
	list->length--;
	list->values=realloc(list->values, list->length*sizeof(void *));
	if(list->length>0 && list->values==NULL){
		printf("VLIST: Memory Error!\n");
		exit(3);
	}
	
	return epos;
}


/* Returns the Value of an element (NULL if Error) */
void *vlist_getvalue(struct s_vlist *list, int index){
	if(list==NULL || index<0 || index>=list->length)
		return NULL;
	return list->values[index];
}

void vlist_setvalue(struct s_vlist *list, int index, void *value){
	if(list==NULL || index<0 || index>=list->length)
		return;
	list->values[index]=value;
}


