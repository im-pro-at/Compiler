/* 
 * BURM Header File
 */

#ifndef _BURM_H_ 
#define _BURM_H_

#include "tree.h"
#include "reg.h"

STATEPTR_TYPE burm_label(treep p);

void burm_reduce(treep bnode, int goalnt, regsp regs);
 



#endif /* _BURM_H_ */
