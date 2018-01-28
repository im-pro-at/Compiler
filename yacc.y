%{

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>	
#include <string.h>

#include "debug.h"
#include "tools.h"
#include "list.h"
#include "lex.h"
#include "yacc.h"
#include "tree.h"
#include "burm.h"
#include "reg.h"

typedef struct label_id{
	int id;
} *labelp; 

labelp new_label(int id){
	labelp label = malloc(sizeof(struct label_id));
	if(label==NULL){
		printf("LABEL: Memory Error!");
		exit(3);
	}
	label->id=id;
	return label;
}

%}

/* Attribut wandert automatisch nach unten v*/
@autoinh inh_labels inh_vars

/* Attribut wandert automatisch nach oben ^ */
@autosyn execute name value syn_labels syn_vars 

@attributes {char *name;} T_IDENT
@attributes {long value;} T_NUM

@attributes {struct s_list *func_names;} Program

@attributes {char *name; struct s_list *syn_labels; struct s_list *syn_vars;} Funcdef

@attributes {char *name;} Pars_b
@attributes {struct s_list *syn_vars;} Pars_a Pars

@attributes {struct s_list *syn_labels; struct s_list *syn_vars;
             struct s_list *inh_labels; struct s_list *inh_vars;} Stats 

@attributes {struct s_list *syn_labels;} Label  

@attributes {struct s_list *syn_labels; char *new_var_name;
             struct s_list *inh_labels; struct s_list *inh_vars; int unique;} Stat

@attributes {struct s_list *inh_vars; bool execute; char *new_var_name; treep n;} SExpr  

@attributes {struct s_list *inh_vars; bool execute; treep n;} Expr Expr_p Expr_m Expr_a Unary Term  

@attributes {struct s_list *inh_vars; int anz; treep n;} Param Param_a Param_b 



%token T_END T_RETURN T_GOTO T_IF T_THEN T_VAR T_NOT T_AND T_EQ_LESS

%token T_IDENT T_NUM 

%start Program


@traversal check		/* Testet ob Variabeln und Labels bei der Verwendung existeiren */
@traversal @preorder test	/* für debug ausgeben */
@traversal @postorder codegen 	/* Generiert den Code */

%%


Program	: 	/* emty */
			@{
				@i @Program.0.func_names@ = list_init();
				@test list_printdebug(@Program.0.func_names@, "#OX: Names of Functions:");
				@codegen @revorder (1) 
					//Liste der Functionen:
					printf("#Functions: \n");
					int i=0;
					for(i=0;i<list_length(@Program.0.func_names@);i++){
						printf(".global %s \n",list_getname(@Program.0.func_names@,i));
					}

			@}
	| 	Program Funcdef ';'  
			{ LD("#Yacc:Reduce: Program\n"); }
			@{
				@i @Program.0.func_names@ = list_tool_addle_unsave(@Program.1.func_names@, @Funcdef.0.name@, NULL); 
			@}	
	;  

Funcdef	: 	T_IDENT '(' Pars ')' Stats T_END			/* Funktionsdefinition */  
			{ LD("#Yacc:Reduce: Function\n"); } 
			@{
				@i @Funcdef.0.syn_vars@ = list_tool_addll(@Pars.0.syn_vars@, @Stats.0.syn_vars@, "Name of Vars / Parameters are the same! ");
				@m Stats.0.inh_labels Stats.0.inh_vars;
					@Stats.0.inh_labels@ = @Funcdef.0.syn_labels@;
					@Stats.0.inh_vars@ = @Funcdef.0.syn_vars@;
					list_tool_exclusive(@Funcdef.0.syn_labels@,@Funcdef.0.syn_vars@,"Name of Label same as Var!");

				@test 
					D("#OX: Function %s: \n", @Funcdef.0.name@); 
					list_printdebug(@Funcdef.0.syn_labels@, "#    -> Labels :"); 
					list_printdebug(@Funcdef.0.syn_vars@, "#    -> Vars :");

				@codegen @revorder (1)
					printf("%s:\n",@Funcdef.0.name@);

				@codegen
					printf("\txorq %%rax, %%rax\n"); 
					printf("\tret\n");

			@}
			
	;  

Pars	: 	Pars_a Pars_b						/* Parameterdefinition */
			{ LD("#Yacc:Reduce: Function Parameter\n"); }
			@{
				@i @Pars.0.syn_vars@ = list_tool_addle(@Pars_a.0.syn_vars@, @Pars_b.0.name@, NULL, "Names of Function Parameters are the same! ");
			@} 
	;
Pars_a	:	/* emty */
			@{
				@i @Pars_a.0.syn_vars@ = list_init();
			@}
	|	Pars_a T_IDENT ','
			@{
				@i @Pars_a.0.syn_vars@ = list_tool_addle(@Pars_a.1.syn_vars@, @T_IDENT.0.name@, NULL, "Names of Function Parameters are the same! ");
			@}
	;
Pars_b	:	/* emty */
			@{
				@i @Pars_b.0.name@ = NULL;
			@}
	|	T_IDENT
	;

Stats	:	/* emty */
			@{
				@i @Stats.0.syn_labels@=list_init();
				@i @Stats.0.syn_vars@=list_init();	
			@}
	|	Stats Label Stat ';'
			{ LD("#Yacc:Reduce: Stats\n");  }
			@{
				@i @Stats.0.syn_labels@ = list_tool_addlll(@Stats.1.syn_labels@, @Label.0.syn_labels@, @Stat.0.syn_labels@, "Tow Labels have the same Name!");
				@i @Stats.0.syn_vars@ = list_tool_addle(@Stats.1.syn_vars@, @Stat.0.new_var_name@, NULL, "Tow Vars have the Same Name!");
			@}
	;

Label	:	/* emty */ 					/* Labeldefinition */  
			@{
				@i @Label.0.syn_labels@ = list_init();
			@}
	|	Label T_IDENT ':'                    
			{ LD("#Yacc:Reduce: Label\n");  }
			@{ 
				@i @Label.0.syn_labels@ = list_tool_addle(@Label.1.syn_labels@, @T_IDENT.name@, new_label(get_unique(true)) , "Tow Labels have the same Name!");
				
				@codegen 
					D("#Label: %s\n",@T_IDENT.name@);
					printf(".L%d:\n",((labelp)list_getvalue(@Label.0.syn_labels@,list_find(@Label.0.syn_labels@, @T_IDENT.name@)))->id);
			@}
	;
 
Stat	: 	T_RETURN Expr  
			{ LD("#Yacc:Reduce: Return expr\n"); }
			@{
				@i @Stat.0.unique@=0;
				@i @Stat.0.syn_labels@=NULL;
				@i @Stat.0.new_var_name@=NULL;

				@codegen
					tree_dprint("Stat [return]", @Expr.0.n@);
					regsp regs=reg_init(@Stat.0.inh_vars@); 
					burm_label(@Expr.0.n@);
					burm_reduce(@Expr.0.n@,1,regs);
					reg_manage(regs,NULL,&@Expr.0.n@->reg,REGM_READ);
					printf("\tmovq %s, %%rax\n",reg_getname(regs,@Expr.0.n@->reg));
					reg_clean(regs,true);
					printf("\tret\n");
			@} 
    	| 	T_GOTO T_IDENT  
			{ LD("#Yacc:Reduce: Goto \n");  }
			@{
				@i @Stat.0.unique@=0;
				@i @Stat.0.syn_labels@=NULL;
				@i @Stat.0.new_var_name@=NULL;

				@check 
					list_tool_isinlist(@Stat.0.inh_labels@, @T_IDENT.name@, "Label is not definde!");

				@codegen
					D("#Label: %s\n",@T_IDENT.name@);
					printf("\tjmp .L%d\n",((labelp)list_getvalue(@Stat.0.inh_labels@,list_find(@Stat.0.inh_labels@, @T_IDENT.name@)))->id);

			@} 
    	| 	T_IF Expr T_THEN Stats T_END  
			{ LD("#Yacc:Reduce: if then end\n"); } 
			@{
				@i @Stat.0.unique@=get_unique(true);
				@i @Stat.0.new_var_name@=NULL;
				
				@m Stats.0.inh_vars;
					@Stats.0.inh_vars@ = list_tool_cpll(@Stat.0.inh_vars@, @Stats.0.syn_vars@, "Name of Var/Parameter are the same!");
					list_tool_exclusive(@Stat.0.inh_labels@, @Stats.0.syn_vars@, "Name of Label same as Var!");

				@test 
					D("#OX:    IF (%d) \n",@Stat.0.unique@); 
					list_printdebug(@Stats.0.inh_labels@, "#       -> Labels :"); 
					list_printdebug(@Stats.0.inh_vars@, "#       -> Vars :");
 
				@codegen @revorder (1)
					/* HEAF */
					D("#IF (%d)  HEAD:\n",@Stat.0.unique@);	
					tree_dprint("Stat [IF_EXPR]", @Expr.0.n@);
					regsp regs=reg_init(@Stat.0.inh_vars@); 
					burm_label(@Expr.0.n@);
					burm_reduce(@Expr.0.n@,1,regs);
					if(reg_is_reg_const(regs,@Expr.0.n@->reg)){
						int val=reg_get_const(regs,@Expr.0.n@->reg);
						reg_free(regs,@Expr.0.n@->reg);
						reg_clean(regs,false);
						if(!(val&0x01)){
							printf("\tjmp .M%d\n",@Stat.0.unique@);
						}
					}
					else
					{
						reg_manage(regs,NULL,&@Expr.0.n@->reg,REGM_READ);
						printf("\tmovq %s, %%rax\n",reg_getname(regs,@Expr.0.n@->reg));
						reg_clean(regs,false);
						printf("\ttestb %s1, %%al\n","$");
						printf("\tje .M%d\n",@Stat.0.unique@);
					}
				@codegen 
					/* TAIL */
					D("#IF (%d) TAIL:\n",@Stat.0.unique@);	
					printf(".M%d:\n",@Stat.0.unique@);
			@} 
	|	SExpr
			{ LD("#Yacc:Reduce: Singel Expr\n"); } 
			@{
				@i @Stat.0.unique@=0;
				@i @Stat.0.new_var_name@=@SExpr.0.new_var_name@;
				@i @Stat.0.syn_labels@=NULL;

				@codegen
					tree_dprint("Stat [Sexpr]", @SExpr.0.n@);
					if(@SExpr.0.execute@){
						regsp regs=reg_init(@Stat.0.inh_vars@); 
						burm_label(@SExpr.0.n@);
						burm_reduce(@SExpr.0.n@,1,regs);
						reg_free(regs,@SExpr.0.n@->reg);
						reg_clean(regs,false);
					}
					else{
						D("#IGNORE SEXPR!\n");
					}
			@}
	;			
    	


SExpr	: 	T_VAR T_IDENT '=' Expr				/* Variablendefinition */  
			{ LD("#Yacc:Reduce: Var def\n");  }
			@{
				@i @SExpr.0.new_var_name@ = @T_IDENT.0.name@;
				@i @SExpr.0.execute@=true; 
				@i @SExpr.0.n@= tree_node_wident(@Expr.0.n@,@T_IDENT.0.name@);				

				@check list_tool_isinlist(@SExpr.0.inh_vars@, @T_IDENT.name@, "Internal Error! Lost Var def!");
			@} 
	|	T_IDENT '=' Expr
			{ LD("#Yacc:Reduce: Zuweisung (IDENT) \n"); } 
			@{
				@i @SExpr.0.new_var_name@=NULL;
				@i @SExpr.0.execute@=true; 
				@i @SExpr.0.n@= tree_node_wident(@Expr.0.n@,@T_IDENT.0.name@);				
				
				@check list_tool_isinlist(@SExpr.0.inh_vars@, @T_IDENT.name@, "Var is not defined!");
			
			@}
	|	'*' Unary '=' Expr				/* schreibender Speicherzugriff */ 
			{ LD("#Yacc:Reduce: Zuweisung (ADRESS) \n"); } 
			@{
				@i @SExpr.0.new_var_name@=NULL;
				@i @SExpr.0.execute@=true; 
				@i @SExpr.0.n@= tree_node_op(NT_WADDRES,@Unary.0.n@,@Expr.0.n@);				

			@}
    	| 	Term  
			{ LD("#Yacc:Reduce: Term \n");  }
			@{
				@i @SExpr.0.new_var_name@=NULL;
				@i @SExpr.0.n@ = @Term.0.n@;
			@} 
    	;  


Expr	:	Unary
			{ LD("#Yacc:Reduce: Expr Unary\n");  }
			@{
				@i @Expr.0.n@=@Unary.0.n@;
			@}
	|	Expr_p '+' Term	
			{ LD("#Yacc:Reduce: Expr +\n");  }
			@{
				@i @Expr.0.execute@= @Expr_p.0.execute@ || @Term.0.execute@;
				@i @Expr.0.n@=tree_node_op(NT_PLUS,@Expr_p.0.n@, @Term.0.n@);
			@}
	|	Expr_m '*' Term	
			{ LD("#Yacc:Reduce: Expr *\n");  }
			@{
				@i @Expr.0.execute@= @Expr_m.0.execute@ || @Term.0.execute@;
				@i @Expr.0.n@=tree_node_op(NT_MUL,@Expr_m.0.n@, @Term.0.n@);
			@}
	|	Expr_a T_AND Term
			{ LD("#Yacc:Reduce: Exp and\n"); } 
			@{
				@i @Expr.0.execute@= @Expr_a.0.execute@ || @Term.0.execute@;
				@i @Expr.0.n@=tree_node_op(NT_AND,@Expr_a.0.n@, @Term.0.n@);
			@}
	|	Term T_EQ_LESS Term
			{ LD("#Yacc:Reduce: Expr =<\n");  }
			@{
				@i @Expr.0.execute@= @Term.0.execute@ || @Term.0.execute@;
				@i @Expr.0.n@=tree_node_op(NT_EQLESS,@Term.0.n@, @Term.1.n@);
			@}
	|	Term '#' Term
			{ LD("#Yacc:Reduce: Expr #\n");  }
			@{
				@i @Expr.0.execute@= @Term.0.execute@ || @Term.0.execute@;
				@i @Expr.0.n@=tree_node_op(NT_UNEQ,@Term.0.n@, @Term.1.n@);
			@}
	;


Expr_p	:	Term
			@{
				@i @Expr_p.0.n@=@Term.0.n@;
			@}
	|	Expr_p '+' Term
			@{
				@i @Expr_p.0.execute@=@Expr_p.1.execute@ || @Term.0.execute@;
				@i @Expr_p.0.n@=tree_node_op(NT_PLUS,@Expr_p.1.n@, @Term.0.n@);
			@}
	;
Expr_m	:	Term
			@{
				@i @Expr_m.0.n@=@Term.0.n@;
			@}
	|	Expr_m '*' Term
			@{
				@i @Expr_m.0.execute@=@Expr_m.1.execute@ || @Term.0.execute@;
				@i @Expr_m.0.n@=tree_node_op(NT_MUL,@Expr_m.1.n@, @Term.0.n@);
			@}
	;
Expr_a	:	Term
			@{
				@i @Expr_a.0.n@=@Term.0.n@;
			@}
	|	Expr_a T_AND Term
			@{
				@i @Expr_a.0.execute@=@Expr_a.1.execute@ || @Term.0.execute@;
				@i @Expr_a.0.n@=tree_node_op(NT_AND,@Expr_a.1.n@, @Term.0.n@);
			@}
	;


Unary	:	T_NOT Unary
			{ LD("#Yacc:Reduce: NOT Unary\n"); } 
			@{
				@i @Unary.0.n@=tree_node_unary(NT_NOT, @Unary.1.n@);
			@}
	|	'-' Unary
			{ LD("#Yacc:Reduce: - Unaray \n"); } 
			@{
				@i @Unary.0.n@=tree_node_unary(NT_MINUS, @Unary.1.n@);
			@}
     	| 	'*' Unary   					/* lesender Speicherzugriff */  
			{ LD("#Yacc:Reduce: * Unaray \n"); } 
			@{
				@i @Unary.0.n@=tree_node_unary(NT_READ, @Unary.1.n@);
			@}
	|	Term
			{ LD("#Yacc:Reduce: Uneray=Term\n"); } 
			@{
				@i @Unary.0.n@=@Term.0.n@;
			@}
	;

 
Term	: 	'(' Expr ')'  
			{ LD("#Yacc:Reduce: (Expr)\n");  }
			@{
				@i @Term.0.n@=@Expr.0.n@;
			@}
    	| 	T_NUM  
			{ LD("#Yacc:Reduce: NUM\n");  }
			@{
				@i @Term.0.execute@=false;
				@i @Term.0.n@=tree_node_const(@T_NUM.0.value@);
			@}
    	| 	T_IDENT						/* Variablenverwendung */  
			{ LD("#Yacc:Reduce: IDENT\n"); } 
			@{
				@i @Term.0.execute@=false;
				@i @Term.0.n@=tree_node_var(@T_IDENT.0.name@);
				@check list_tool_isinlist(@Term.0.inh_vars@, @T_IDENT.name@, "Var is not defined!");
			@} 
    	| 	T_IDENT '(' Param  ')' 				/* Funktionsaufruf */  
			{ LD("#Yacc:Reduce: Funktion\n"); }
			@{
				@i @Term.0.execute@=true;
				@i @Term.0.n@=tree_node_call(@Param.0.n@, @Param.0.anz@, @T_IDENT.0.name@);	
			@} 
	;

Param	:	Param_a Param_b
			@{
				@i @Param.0.anz@=@Param_a.0.anz@+@Param_b.0.anz@;
				@m Param.0.n;
					if(@Param_b.0.n@==NULL)
						@Param.0.n@=@Param_a.0.n@;
					else
						@Param.0.n@=tree_node_param(NT_FPARAM_REG, @Param.0.anz@, @Param_a.0.n@, @Param_b.0.n@);
						
			@}
	;
Param_a	:	/* emty */
			@{
				@i @Param_a.0.anz@=0;
				@i @Param_a.0.n@=tree_node_param(NT_FPARAM_START,0,NULL,NULL);
			@}
	|	Param_a Expr ','
			@{
				@i @Param_a.0.anz@=@Param_a.1.anz@+1;
				@i @Param_a.0.n@=tree_node_param(NT_FPARAM_REG, @Param_a.0.anz@, @Param_a.1.n@, @Expr.0.n@);
			@}
	;
Param_b	:	/* emty */
			@{
				@i @Param_b.0.anz@=0;
				@i @Param_b.0.n@=NULL;
			@}
	|	Expr
			@{
				@i @Param_b.0.anz@=1;
				@i @Param_b.0.n@=@Expr.0.n@;
			@}
	;





%%

main(){
	if(yyparse())
		exit(2);
}

yyerror(char* s){
	printf("YACC: error line %d message %s \n",lexZeile, s);
	exit(2);
}


