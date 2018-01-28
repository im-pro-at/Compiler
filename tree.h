/* 
 * Syntax Baum erzeugung 
 */

#ifndef _TREE_H 
#define _TREE_H

#ifndef IN_BURM
typedef struct brum_state *STATEPTR_TYPE;
#endif /* IN_BRUM */

enum NOTE_TYPE 	{ 
			//leavs	
			//0       0
			NT_VAR=1, NT_CONST, NT_CONST_0, NT_CONST_1, NT_CONST_SF,
			//Function
			//1       0                2
			NT_FCALL, NT_FPARAM_START, NT_FPARAM_REG, 
			//Unaray
			//1     1         1
			NT_NOT, NT_MINUS, NT_READ,
			//OP
			//2      2       2       2          2
			NT_PLUS, NT_MUL, NT_AND, NT_EQLESS, NT_UNEQ,
			//LExpr
			//2         1
			NT_WADDRES, NT_WIDENT
		};

struct tree {
	//brum
	enum NOTE_TYPE op;
	struct tree *nodes[2];
	STATEPTR_TYPE state;  
	//User
	long val;
	char* name;
	//Parse
	int reg;
	int rsf;
	int sf;
	int fcall;	
};

typedef struct tree *treep;


void tree_dprint(char* text, treep tree);
treep tree_node_const(long value);
treep tree_node_var(char* name);
treep tree_node_op(enum NOTE_TYPE op, treep left, treep right);
treep tree_node_unary(enum NOTE_TYPE op, treep tree);
treep tree_node_call(treep param, int anz, char* name);
treep tree_node_param(enum NOTE_TYPE op, int pos, treep left, treep right);
treep tree_node_wident(treep expr, char* name);

#endif /* _TREE_H */

