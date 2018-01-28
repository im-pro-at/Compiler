/* 
 * some usefule Functions
 */

#include "tools.h"

int max(int a, int b){
	if(a>b)
		return a;
	return b;
}

int max3(int a , int b, int c){
	return max(a,max(b,c));
}

volatile int get_unique(bool newval){
	static int unique =0;

	if(newval)
		unique++;
	return unique;
} 
