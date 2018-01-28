/*
 * Tool zur Registerbelegung 
 */
#ifndef _REG_H_
#define _REG_H_

#include "list.h"
#include <stdbool.h>

typedef struct reg_state *regsp;

enum REG_MANAGE_OPTIONS{
	/* Für Befehle mit nur einem Register */
	REGM_ONE,
	/* Für den Befhel witd nur ein Register Leden verwendet */
	REGM_READ,
	/* Für Befehle mit zwei registern (operation lässte aber vertauschen zu) */
	REGM_EXCHANGE, 
	/* Für Befehle mit zwei Registern die kein Vertauschen zulässt */
	REGM_FIXED, 
	/* Register müssen wirklich Registern sein (load and lea) */
	REGM_ONLY
};

regsp reg_init(struct s_list *vars);
void reg_clean(regsp regs, bool sp_only);

int reg_link_var(regsp regs, char *name);
int reg_link_const(regsp regs, long value);
bool reg_is_reg_const(regsp regs, int reg);
int reg_get_const(regsp regs, int reg);

int reg_manage(regsp regs, int *src_dest, int *src, enum REG_MANAGE_OPTIONS option);
void reg_free(regsp regs, int reg);
char *reg_getname(regsp regs, int reg);
char *reg_getname_8bit(regsp regs, int reg);
bool reg_last_exchange(regsp regs);

int reg_call_new(regsp regs);
int reg_call_push(regsp regs, int call, int pos, int reg);
int reg_call_do(regsp regs, int call, int anz, char* name);


#endif /* _REG_H_ */
