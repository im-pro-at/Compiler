/*
 * Tool zur Registerbelegung 
 */
#include <stdlib.h>
#include <stdio.h>
#include "reg.h"
#include "debug.h"
#include "tools.h"
#include "list.h"
#include "vlist.h"

#define REG_ANZ 8
#define MAX_FPARAM 6

char *phreg_names[]      ={"%rdi","%rsi","%rdx","%rcx","%r8", "%r9", "%r10", "%r11" };
char *phreg_8bit_names[] ={"%dil","%sil","%dl", "%cl", "%r8b","%r9b","%r10b","%r11b" };

typedef struct reg_phregs_value *phregp;
typedef struct reg_vars_value *varp;
typedef struct reg_regs_value *regp;
typedef struct reg_stack_value *stackp;
typedef struct reg_func_value *funcp;
typedef struct reg_func_value_data *funcrp;

struct reg_state{
	/* Abbildung des verwendeten Stacks */
	struct s_vlist *stack; /* verwendet reg_stack_value */

	/* liste der Varaiblen */
	struct s_list *vars; /* verwendet reg_vars_value */

	/* Liste der Physikalischen Register */
	struct reg_phregs_value {
		char *name;
		char *name_8bit;
		enum {
			RSRS_FREE, RSRS_VAR, RSRS_REG, RSRS_FCALL	
		} status;		
		struct reg_vars_value *var;
		struct reg_regs_value *reg;
		struct reg_func_value_data *funcr;

	} phregs[REG_ANZ];

	/* Liste der gerade verwendeten Register (Freigegebene werden nicht gelöscht!) */
	struct s_vlist *regs; /* verwendet reg_regs_value */

	/* Liste der Functionsaufruf vorbereitungen (Aufgerufene Functionen werden nicht gelöscht!) */
	struct s_vlist *func; /* verwendet reg_func_value */
	
	bool last_exchange;
};

struct reg_vars_value{
	/* Status der Variable */
	enum {
		RVV_INREG, RVV_INMEM
	} status;	
	/* POINTERS */
	struct reg_phregs_value *phreg;
	struct reg_stack_value *stack;
};

struct reg_regs_value{
	int index;	
	char name[20];
	/* gibt an wo die Daten liegen */
	enum reg_regs_value_status{
		RRV_FREED, RRV_VARP, RRV_CONST, RRV_INREG, RRV_INMEM, RRV_INRAX
	} status;
	/* if  linked to const */
	long value;
	/* POINTERS */
	struct reg_vars_value *var;
	struct reg_phregs_value *phreg;
	struct reg_stack_value *stack;
};

struct reg_stack_value{
	/* Legt die Quelle fest */
	enum {
		RSV_FREE, RSV_VAR, RSV_REG, RSV_FCALL
	} status;
	/* POINTERS */
	struct reg_vars_value *var;
	struct reg_regs_value *reg;
	struct reg_func_value_data *funcr;
	
};

struct reg_func_value{
	int index;
	/* Status des Functionsaufrufes */
	enum{
		RFV_OPEN, RFV_DONE
	} status;

	/*Anzahl der bis jetzt benützten Register */
	int count;	

	/* Status der Aufrufregister */
	struct reg_func_value_data {
		struct reg_func_value *func;
		enum{
			RFVD_OPEN, RFVD_INREG, RFVD_INMEM	
		} status;
		/* POINTERS */
		struct reg_stack_value *stack;
		struct reg_phregs_value *phreg;
	} data[MAX_FPARAM];
};

static void error(char *text){
	printf("REG: %s\n",text);
	exit(3);
}

static phregp get_phreg(regsp regs, int index){
	return &regs->phregs[index];
}

static varp get_var(regsp regs, int index){
	return (varp)list_getvalue(regs->vars,index);
}

static regp get_reg(regsp regs, int index){
	regp reg=(regp)vlist_getvalue(regs->regs,index);
	if(reg==NULL)
		error("LOST REG!");
	if(index!=reg->index)
		error("REGINDEX ERROR!");
	return reg;
}

static stackp get_stack(regsp regs, int index){
	return (stackp)vlist_getvalue(regs->stack,index);
}

static int get_stackoffset(regsp regs, struct reg_stack_value *value){
	int index=vlist_find(regs->stack,value);
	if(index==-1)
		error("LOST STACK DEF!");
	return (vlist_length(regs->stack)-1-index)*8;
}

static funcp get_func(regsp regs, int index){
	struct reg_func_value *func= (struct reg_func_value *) vlist_getvalue(regs->func,index);
	if(func==NULL)
		error("LOST FUNC!");
	if(index!=func->index)
		error("FUNCINDEX ERROR!");
	return func;
}

static void *get_memory(int size){
	void *temp=malloc(size);
	if(temp==NULL) 
		error("Memory Error!");
	return temp;
}

/* Initialisiert die Registerverwaltung */
regsp reg_init(struct s_list *vars){
	int i;
	regsp regs = get_memory(sizeof(struct reg_state));

	regs->vars=list_copy(vars);
	if(list_length(regs->vars)>8)
		error("More than 8 Variables!");
	
	for(i=0; i< list_length(regs->vars);i++){
		varp value= get_memory(sizeof(struct reg_vars_value));
		value->status=RVV_INREG;	
		value->phreg=&regs->phregs[i];
		list_setvalue(regs->vars,i,value);	
	}
	
	for(i=0;i <REG_ANZ;i++){
		regs->phregs[i].name=phreg_names[i];
		regs->phregs[i].name_8bit=phreg_8bit_names[i];
		if(i<list_length(regs->vars)){
			regs->phregs[i].status=RSRS_VAR;
			regs->phregs[i].var=get_var(regs,i);
		}
		else
		{
			regs->phregs[i].status=RSRS_FREE;
		}
	}

	regs->regs= vlist_init();
	regs->stack=vlist_init();
	regs->func= vlist_init();
	regs->last_exchange=false;
	return regs;
};

/* Stellt den Uhrzustand wieder her  (greift rax nicht an!)*/
void reg_clean(regsp regs, bool sp_only){
	int i;
	bool only_vars=!sp_only;
	/* Zustand Testen */
	for(i=0;i<REG_ANZ;i++){
		if(get_phreg(regs,i)->status!=RSRS_FREE && get_phreg(regs,i)->status!=RSRS_VAR){
			D("# PHREG NOT FREED status=%d",get_phreg(regs,i)->status);
			error("PHREG NOT FREED!");
		}
	}
	for(i=0;i<vlist_length(regs->regs);i++){
		if(get_reg(regs,i)->status!=RRV_FREED)
			error("REG NOT FREED!");
	}
	for(i=0;i<vlist_length(regs->stack);i++){
		if(get_stack(regs,i)->status!=RSV_FREE && get_stack(regs,i)->status!=RSV_VAR)
			error("STACK NOT FREED!");
		if(get_stack(regs,i)->status!=RSV_VAR)
			only_vars=false;
	}
	if(vlist_length(regs->func) >0)
		error("FUNC NOT DONE!");

	if(only_vars){
		for(i=vlist_length(regs->stack)-1;i>=0;i--){
			char *name=NULL;
			int j;
			for(j=0;j<list_length(regs->vars);j++){
				if(get_var(regs,j)==get_stack(regs,i)->var)
					name=list_getname(regs->vars,j);	
			}
			D("#REG: RECOVER %s\n", name);
			printf("\tpopq %s\n",get_stack(regs,i)->var->phreg->name);	
		}
	}
	else{
		if(!sp_only){
			for(i=0;i<list_length(regs->vars);i++){
				if(get_var(regs,i)->status==RVV_INMEM)
					printf("\tmovq %d(%%rsp), %s\n",get_stackoffset(regs, get_var(regs,i)->stack),get_var(regs,i)->phreg->name);
			}	
		}
		if(vlist_length(regs->stack)>0){
			printf("\taddq $%d, %%rsp\n",vlist_length(regs->stack)*8);
		}		
	}
};

/* Gibt ein Register zurück das die Variable beinhaltet */
int reg_link_var(regsp regs, char *name){
	regp reg=get_memory(sizeof(struct reg_regs_value));
	int varindex=list_find(regs->vars,name);
	reg->index=vlist_add(regs->regs,reg);
	reg->status=RRV_VARP;
	if(varindex==-1)
		error("LOST VARNAME IN REGSYSTEM!");
	reg->var=get_var(regs,varindex);
	D("#REG: ALLOC %d: Link VAR %s \n",reg->index,name);
	return reg->index;
}

/* GIbt ein Register zurück das die Constante beinhaltet */
int reg_link_const(regsp regs, long value){
	regp reg=get_memory(sizeof(struct reg_regs_value));
	reg->index=vlist_add(regs->regs,reg);
	reg->status=RRV_CONST;
	reg->value=value;
	D("#REG ALLOC %i: Link CONST %ld \n",reg->index,value);
	return reg->index;
}

bool reg_is_reg_const(regsp regs, int reg){
	return (get_reg(regs,reg)->status==RRV_CONST);
}

int reg_get_const(regsp regs, int reg){
	if(get_reg(regs,reg)->status!=RRV_CONST)
		error("ILLEGALE CONST ABFRAGE!");
	return get_reg(regs,reg)->value;
}

/* gitb zurück ob push (tre) oder mov (false) */
bool get_free_stack_entry(regsp regs, stackp *stack){
	int i;
	for(i=0; i< vlist_length(regs->stack);i++){
		*stack=(stackp)vlist_getvalue(regs->stack,i);
		if((*stack)->status==RSV_FREE){
			return false;
		}
	}
	*stack=get_memory(sizeof(struct reg_stack_value));
	vlist_add(regs->stack,*stack);
	return true;
}


static void free_phreg(regsp regs, phregp phreg){
	switch (phreg->status){
		stackp stack;
		case RSRS_FREE:
			/* REG IST SCHON FREI */
			break;
		case RSRS_REG:
			if(phreg->reg->status!=RRV_INREG || phreg->reg->phreg!=phreg)
				error("VAR POINTER TO PHREG ERROR!");

			if(get_free_stack_entry(regs,&stack))
				printf("\tpushq %s\n",phreg->name);
			else
				printf("\tmovq %s, %d(%%rsp)\n",phreg->name, get_stackoffset(regs, stack));
			phreg->status = RSRS_FREE;
			phreg->reg->status = RRV_INMEM;
			phreg->reg->stack = stack;
			stack->status = RSV_REG;
			stack->reg = phreg->reg;
			break;
		case RSRS_VAR:
			if(phreg->var->status!=RVV_INREG || phreg->var->phreg!=phreg)
				error("VAR POINTER TO PHREG ERROR!");

			if(get_free_stack_entry(regs,&stack))
				printf("\tpushq %s\n",phreg->name);
			else
				printf("\tmovq %s, %d(%%rsp)\n",phreg->name, get_stackoffset(regs, stack));
			
			phreg->status = RSRS_FREE;
			phreg->var->status = RVV_INMEM;
			phreg->var->stack = stack;
			stack->status = RSV_VAR;
			stack->var = phreg->var;
			break;
		case RSRS_FCALL:
			D("#Must use a FCALL PHREG!\n");
			if(phreg->funcr->status!=RFVD_INREG || phreg->funcr->func->status!=RFV_OPEN || phreg->funcr->phreg!=phreg)
				error("FUNCR POINTER TO PHREG ERROR");
			
			if(get_free_stack_entry(regs,&stack))
				printf("\tpushq %s\n",phreg->name);
			else
				printf("\tmovq %s, %d(%%rsp)\n",phreg->name, get_stackoffset(regs, stack));
			
			phreg->status = RSRS_FREE;
			phreg->funcr->status = RFVD_INMEM;
			phreg->funcr->stack = stack;
			stack->status = RSV_FCALL;
			stack->funcr = phreg->funcr;
			break;
		default:
			error("REGISTER KANN NICHT FREIGERAEMT WERDEN!");
	}
}

static void alloc_reg(regsp regs, regp reg, regp ravoid){
	D("# ALLOC: REG: Index=%d ADRESS=%p AVOID: %p\n", reg->index, reg, ravoid);
	int i;
	phregp phregavoid=NULL;
	if(ravoid!=NULL && ravoid->status==RRV_INREG)
		phregavoid=ravoid->phreg;
	if(reg->status==RRV_INREG){
		/* Register ist wirklich ein Register :-) */
		if(reg->phreg->status!=RSRS_REG || reg->phreg->reg!=reg)
			error("PHREG POINTER TO REG ERROR!");
		return;
	}

	if(reg->status==RRV_VARP && reg->var->status==RVV_INREG){
		/* Register ist ein Pointer auf eine Variable die noch im Register liegt */
		D("#  REGMEM TO STACK reg: %p \n",reg);
		stackp stack;
		if(get_free_stack_entry(regs,&stack))
			printf("\tpushq %s\n",reg->var->phreg->name);
		else
			printf("\tmovq %s, %d(%%rsp)\n",reg->var->phreg->name, get_stackoffset(regs, stack));
		reg->status = RRV_INREG;
		reg->phreg = reg->var->phreg;
		reg->phreg->status = RSRS_REG;
		reg->phreg->reg = reg;
		reg->var->status = RVV_INMEM;
		reg->var->stack = stack;
		stack->status = RSV_VAR;
		stack->var = reg->var;

		return;
	}

	/* find free reg */
	phregp phreg=NULL;
	for(i=0;i<REG_ANZ;i++){
		if(get_phreg(regs,i)->status==RSRS_FREE && phregavoid!=get_phreg(regs,i)){
			phreg=get_phreg(regs,i);
			break;
		}
	}	
	if(phreg==NULL){
		for(i=0;i<REG_ANZ;i++){
			if(get_phreg(regs,i)->status==RSRS_VAR && phregavoid!=get_phreg(regs,i)){
				phreg=get_phreg(regs,i);
				break;
			}
		}
	}
	if(phreg==NULL){
		for(i=0;i<REG_ANZ;i++){
			if(get_phreg(regs,i)->status==RSRS_REG && phregavoid!=get_phreg(regs,i)){
				phreg=get_phreg(regs,i);
				break;
			}
		}
	}

	/* Register wird schon benützt => feiräumen */
	free_phreg(regs, phreg);

	/* Daten Laden */
	switch (reg->status){
		case RRV_VARP:
				if(reg->var->status==RVV_INREG)
					printf("\tmovq %s, %s\n",reg->var->phreg->name,phreg->name);
				else
					printf("\tmovq %d(%%rsp), %s\n",get_stackoffset(regs,reg->var->stack),phreg->name);
			break;
		case RRV_CONST:
				printf("\tmovq $%ld, %s\n",reg->value, phreg->name);
			break;
		case RRV_INMEM:
				if(reg->stack->status!=RSV_REG || reg->stack->reg!=reg)
					error("STACK POINTER TO REG ERROR!");

				if(get_stackoffset(regs,reg->stack)==0){
					printf("\tpopq %s\n", phreg->name);
					vlist_remove(regs->stack,reg->stack);	
				}				
				else{
					printf("\tmovq %d(%%rsp), %s\n", get_stackoffset(regs,reg->stack),phreg->name);
					reg->stack->status = RSV_FREE;
				}
			break;
		case RRV_INRAX:
			printf("\tmovq %%rax, %s\n",phreg->name);
			break;
		default:
			error("REG DATE INVALIDE!");
	}
	reg->status = RRV_INREG;
	reg->phreg = phreg;
	phreg->status = RSRS_REG;
	phreg->reg = reg;

}


static void calc_reg_name(regsp regs, regp reg){
	D("# CALCNAME: %p \n",reg);
	switch (reg->status){
		case RRV_VARP:
			if(reg->var->status==RVV_INREG){
				if(reg->var->phreg->status!=RSRS_VAR || reg->var->phreg->var!=reg->var)
					error("PHREG POINTER TO VAR ERROR!");

				sprintf(reg->name,"%s",reg->var->phreg->name);
			}
			else{
				if(reg->var->stack->status!=RSV_VAR || reg->var->stack->var!=reg->var)
					error("STACK PONTER TO VAR ERROR!");

				sprintf(reg->name,"%d(%%rsp)",get_stackoffset(regs, reg->var->stack));
			}
			break;
		case RRV_CONST:
			sprintf(reg->name,"$%ld",reg->value);
			break;
		case RRV_INREG:
			if(reg->phreg->status!=RSRS_REG || reg->phreg->reg!=reg)
				error("PHREG POINTER TO REG ERROR!");

			sprintf(reg->name,"%s",reg->phreg->name);
			break;
		case RRV_INMEM:
			if(reg->stack->status!=RSV_REG || reg->stack->reg!=reg)
				error("STACK POINTER TO REG ERROR!");

			sprintf(reg->name,"%d(%%rsp)",get_stackoffset(regs, reg->stack));
			break;
		default:
			error("NAMING A FREE REGISTER!");
		}		
}

int reg_manage(regsp regs, int *irsd, int *irs, enum REG_MANAGE_OPTIONS option){	
	D("#REG: MENAGE:\n");
	if(option != REGM_READ)
		D("# REG SD: INDEX=%d ADRESS=%p SATUS=%d \n",get_reg(regs,*irsd)->index, get_reg(regs,*irsd), get_reg(regs,*irsd)->status);
	if(option != REGM_ONE)
		D("# REG S: INDEX=%d ADRESS=%p SATUS=%d \n",get_reg(regs,*irs)->index, get_reg(regs,*irs), get_reg(regs, *irs)->status);
	

	regp rsd, rs;
	/* Intelligenter tausch */
	regs->last_exchange=false;
	if(option==REGM_EXCHANGE ){
		enum reg_regs_value_status srsd, srs;
		srsd=get_reg(regs,*irsd)->status;
		if(srsd==RRV_VARP && get_reg(regs,*irsd)->var->status==RVV_INMEM)
			srsd=RRV_INMEM;	
		srs=get_reg(regs,*irs)->status;
		if(srs==RRV_VARP && get_reg(regs,*irs)->var->status==RVV_INMEM)
			srs=RRV_INMEM;
		D("# srsd=%d, srs=%d\n",srsd, srs);
 		switch(srsd){
			case RRV_INREG:
				/* Tauscht nicht! */
				break;		
			case RRV_CONST:
			case RRV_INMEM:
				if (srs==RRV_INREG || srs==RRV_VARP)
					regs->last_exchange=true;	
				break;
			case RRV_VARP:
				if (srs==RRV_INREG)
					regs->last_exchange=true;	
				break;
			default:
				error("REG IS FREE!");
		}
	}
	if(regs->last_exchange==true){
		D("# EXCHANGE!");
		/* Register werden ausgetauscht */
		int temp=*irsd;
		*irsd=*irs;
		*irs=temp;
	}	

	if(option!=REGM_READ)	
		rsd=get_reg(regs,*irsd);
	else
		rsd=NULL;

	if(option!=REGM_ONE)
		rs=get_reg(regs,*irs);
	else
		rs=NULL;

	if((rsd==NULL && option!=REGM_READ) || (rs==NULL && option!=REGM_ONE))
		error("REG NULLPOINTER!");

	/* rsd muss in einem Register liegen! */
	if(option!=REGM_READ)
		alloc_reg(regs,rsd,rs);
		
	/* Namen Berechnen */
	if(option!=REGM_READ)
		calc_reg_name(regs,rsd);

	if(option!=REGM_ONE){
		calc_reg_name(regs,rs);	
		if(option==REGM_ONLY && !(rs->status==RRV_INREG || (rs->status==RRV_VARP && rs->var->status==RVV_INREG)) ){
			D("# in Temp rs->satus=%d \n",rs->status);
			printf("\tmovq %s, %%rax\n",rs->name);
			sprintf(rs->name,"%s","%rax");
		}
	}

	/* rs freigeben */
	if(option!=REGM_ONE)
		reg_free(regs, rs->index);	

	D("#USED s_d=%d, s=%d \n",option==REGM_READ?-1:rsd->index,option==REGM_ONE?-1:rs->index);

	if(option!=REGM_READ)
		return rsd->index;
	else
		return -1;
} 


/* gibt ein benütztes Register wieder frei */
void reg_free(regsp regs, int ireg){
	D("#REG: free %d \n",ireg);

	regp reg= get_reg(regs,ireg);
	if(reg==NULL)
		error("FREEING NULL POINTER");

	switch(reg->status){
		case RRV_VARP:
		case RRV_CONST:
			/* nichts zu tun! */
			break;
		case RRV_INREG:
			if(reg->phreg->status!=RSRS_REG || reg->phreg->reg!=reg)
				error("PHREG TO REG POINTER ERROR!");
			
			reg->phreg->status=RSRS_FREE;			
			
			break;

		case RRV_INMEM:
			if(reg->stack->status!=RSV_REG || reg->stack->reg!=reg)
				error("STACK TO REG POINTER ERROR!");

			reg->stack->status=RSV_FREE;
			
			break;

		default:
			error("CANNOT FREE REG!");
	}	
	reg->status=RRV_FREED;
}

/* gibt den Bezeichner des Registers Zurück (diser verfällt nach dem übernächsten aufruf der function) */
char *reg_getname(regsp regs, int reg){
	return get_reg(regs,reg)->name;
}

char *reg_getname_8bit(regsp regs, int reg){
	if(get_reg(regs,reg)->status!=RRV_INREG)
		error("NO 8bit NAME FOR UNALLOCEDED REG!");
	return get_reg(regs,reg)->phreg->name_8bit;
}

bool reg_last_exchange(regsp regs){
	return regs->last_exchange;
}

int reg_call_new(regsp regs){
	int i;
	funcp func= get_memory(sizeof(struct reg_func_value));
	func->index=vlist_add(regs->func,func);
		
	func->status=RFV_OPEN;
	func->count=0;

	for(i=0;i<MAX_FPARAM;i++){
		func->data[i].func=func;
		func->data[i].status=RFVD_OPEN;
	}

	D("#OPEN FUNCTION %d\n",func->index);
	return func->index;
}
int reg_call_push(regsp regs, int call, int pos, int ireg){
	D("#REG PUSH %d Func=%d\n", pos,call);	
	funcp func=get_func(regs,call);
	if(pos>MAX_FPARAM)
		error("Too many funccall parameter!");
	if((vlist_length(regs->func)-1)!=call)
		error("Mixed up tow function calls!");
	if((func->count+1)!=pos)
		error("Mixed up parameter order!");
	if(func->status!=RFV_OPEN)
		error("Function is not open!");

	regp reg = get_reg(regs,ireg);
	funcrp funcr = &func->data[func->count];
	phregp phreg = &regs->phregs[pos-1];

	D("# reg status %d, reg->phreg %p == %p\n",reg->status,reg->phreg, phreg);
	if(reg->status==RRV_INREG && reg->phreg==phreg){
		/* reg == Phreg */
		reg->status = RRV_FREED;
		funcr->status = RFVD_INREG;
		funcr->phreg = phreg;
		phreg->status = RSRS_FCALL;
		phreg->funcr = funcr;
	}
	else if(reg->status==RRV_VARP && reg->var->status==RVV_INREG && reg->var->phreg==phreg){
		/* free Phreg */
		free_phreg(regs,phreg);
		
		reg->status = RRV_FREED;
		funcr->status = RFVD_INREG;
		funcr->phreg = phreg;
		phreg->status = RSRS_FCALL;
		phreg->funcr = funcr;
	}
	else{	
		/* free Phreg */
		free_phreg(regs,phreg);
		
		funcr->status = RFVD_INREG;
		funcr->phreg = phreg;
		phreg->status = RSRS_FCALL;
		phreg->funcr = funcr;
	
		/* reg vorbereiten und freigeben */
		reg_manage(regs, NULL, &ireg, REGM_READ);
	
		if(phreg->status!=RSRS_FCALL)
			error("Internal Error (Menage used Fcall phreg)!");
	
		/* move data */
		printf("\tmovq %s, %s\n",reg_getname(regs, ireg),phreg->name);
	}
		
	func->count++;
	return func->index;
}
int reg_call_do(regsp regs, int call, int anz, char* name){
	D("#DO FuncCALL=%d\n", call);
	int i;	
	funcp func = get_func(regs,call);			
		
	if((vlist_length(regs->func)-1)!=call)
		error("Mixed up tow function calls!");
	if(func->count!=anz)
		error("parameter count error!");
	if(func->status!=RFV_OPEN)
		error("Function is not open!");
	
	for(i=0;i<func->count;i++){
		if(func->data[i].status!=RFVD_INREG || func->data[i].phreg->status!=RSRS_FCALL || func->data[i].phreg->funcr!=&func->data[i])
			error("Lost Func Call Register Data!");
	}	
	
	/* übrige Register Sichern */
	for(i=func->count;i<REG_ANZ;i++)
		free_phreg(regs, &regs->phregs[i]);

	/* call */
	printf("\tcall %s\n",name);

	/* Call register freigeben */
	for(i=0;i<func->count;i++)
		get_phreg(regs,i)->status=RSRS_FREE;
	
	/* func aus liste löschen */
	vlist_remove(regs->func,func);
	
	/* Callregister der darunterliegenden aufrufes wiederherstellen */
	if(call>0){
		func=get_func(regs,call-1);
		for(i=0;i<func->count;i++){
			phregp phreg=get_phreg(regs,i);
			funcrp funcr=&func->data[i];			
			if(funcr->status!=RFVD_INMEM || phreg->status!=RSRS_FREE)
				error("Problem loading Registers after a Call!");			

			if(funcr->stack->status!=RSV_FCALL || funcr->stack->funcr!=funcr)
				error("STACK POINTER TO FUNCR ERROR!");

			if(get_stackoffset(regs,funcr->stack)==0){
				printf("\tpopq %s\n", phreg->name);
				vlist_remove(regs->stack,funcr->stack);	
			}				
			else{
				printf("\tmovq %d(%%rsp), %s\n", get_stackoffset(regs,funcr->stack),phreg->name);
				funcr->stack->status = RSV_FREE;
			}
			funcr->status = RFVD_INREG;
			funcr->phreg = phreg;
			phreg->status = RSRS_FCALL;
			phreg->funcr = funcr;
		}
	}
	
	/* Daten übernehmen */	
	
	regp reg=get_memory(sizeof(struct reg_regs_value));
	reg->index=vlist_add(regs->regs,reg);
	reg->status=RRV_INRAX;
	D("#REG: ALLOC %d: INRAX \n",reg->index);

	alloc_reg(regs, reg, NULL);

	return reg->index;
}

