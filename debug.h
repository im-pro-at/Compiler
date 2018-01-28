/* 
 * Debug System 
 */

#ifndef _DEBUG_H 
#define _DEBUG_H

#ifdef LDEBUG
#define DEBUG
#endif

#ifdef LDEBUG 
#define LD(...) printf(__VA_ARGS__)
#else
#define LD(...) ((void)0)
#endif

#ifdef DEBUG 
#define D(...) printf(__VA_ARGS__)
#else
#define D(...) ((void)0)
#endif


#endif /* _DEBUG_H */



