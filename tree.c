/* 
 * Syntax Baum erzeugung 
 */
#include <stdlib.h>
#include <stdio.h>
#include "tree.h"

static treep tree_new_node(enum NOTE_TYPE op, treep left, treep right, long val, char* name){
	treep temp = malloc(sizeof(struct tree));
	if(temp==NULL) {
		printf("TREE: Memory Error!\n");
		exit(3);	
	}

	temp->op=op;
	temp->nodes[0]=left;
	temp->nodes[1]=right;
	temp->val =val;
	temp->name=name;
	temp->reg=-1;
	temp->rsf=-1;
	temp->sf=0;
	return temp;
}


static void tprint(treep tree, int level);

void tree_dprint(char* text, treep tree){
	#ifndef DEBUG
		return;
	#endif
	
	if(text!=NULL) 
		printf("#TREE: %s: \n",text);
	
	tprint(tree,0);
}

static void dprint_linestart(int level){
	int i=0;
	printf("#   ");
	for(i=0;i<level;i++)
		printf("`   ");
}

static void tprint(treep tree, int level){
	
	if(tree==NULL){
		dprint_linestart(level);
		printf("NULL\n");
	}
	else{		
		dprint_linestart(level);
		switch (tree->op){
			case NT_CONST:
				printf("CONST{%ld}",tree->val); 
				break;	
			case NT_CONST_0:
				printf("CONST_0{%ld}",tree->val); 
				break;	
			case NT_CONST_1:
				printf("CONST_1{%ld}",tree->val); 
				break;	
			case NT_CONST_SF:
				printf("CONST_SF{%ld}",tree->val); 
				break;	
			case NT_VAR:
				printf("VAR{%s}",tree->name); 
				break;	
			case NT_FPARAM_START:
				printf("FPS"); 
				break;	
			case NT_FPARAM_REG:
				printf("FPR{%ld}",tree->val); 
				break;	
			case NT_FCALL:
				printf("FCALL{%s}",tree->name); 
				break;	
			case NT_NOT:
				printf("!");
				break;	
			case NT_MINUS:
				printf("-"); 
				break;	
			case NT_READ:
				printf("READ"); 
				break;	
			case NT_PLUS:
				printf("+"); 
				break;	
			case NT_MUL:
				printf("*"); 
				break;	
			case NT_AND:
				printf("&"); 
				break;	
			case NT_EQLESS:
				printf("=<"); 
				break;	
			case NT_UNEQ:
				printf("#"); 
				break;	
			case NT_WADDRES:
				printf("WRITE"); 
				break;	
			case NT_WIDENT:
				printf("WRITE{%s}",tree->name); 
				break;	
			default:
				printf("?");
				break;

		}	
		if(tree->nodes[0]!=NULL || tree->nodes[1]!=NULL)
			printf("(\n");
		else
			printf("\n");

		if(tree->nodes[0]!=NULL)
			tprint(tree->nodes[0],level+1);

		if(tree->nodes[1]!=NULL)
			tprint(tree->nodes[1],level+1);
		if(tree->nodes[0]!=NULL || tree->nodes[1]!=NULL){
			dprint_linestart(level);
			printf(")\n");
		}
	}

}


treep tree_node_const(long value){
	switch(value){
		case 0:
			return tree_new_node(NT_CONST_0, NULL, NULL, value, NULL);
			break;
		case 1:
			return tree_new_node(NT_CONST_1, NULL, NULL, value, NULL);
			break;
		case 2:
		case 4:
		case 8:
			return tree_new_node(NT_CONST_SF, NULL, NULL, value, NULL);
			break;
		default:
			return tree_new_node(NT_CONST, NULL, NULL, value, NULL);
			break;
	}
}

treep tree_node_var(char* name){
	return tree_new_node(NT_VAR, NULL, NULL, 0, name);
}

treep tree_node_op(enum NOTE_TYPE op, treep left, treep right){
	return tree_new_node(op, left, right, 0, NULL);
} 

treep tree_node_unary(enum NOTE_TYPE op, treep tree){
	return tree_new_node(op, tree, NULL, 0, NULL);
} 

treep tree_node_call(treep param, int anz, char* name){
	return tree_new_node(NT_FCALL,param,NULL,anz,name);
}

treep tree_node_param(enum NOTE_TYPE op, int pos, treep left, treep right){
	return tree_new_node(op, left, right, pos, NULL);
}

treep tree_node_wident(treep expr, char* name){
	return tree_new_node(NT_WIDENT,expr,NULL,0,name);
}

