
buch	[a-zA-Z]
digit	[0-9]
hex	[0-9a-fA-F]

%{
	#include <string.h>
	#include <stdbool.h>	

	#include "debug.h"		
	#include "lex.h"
	#include "yacc.h"
	#include "tree.h"

	#define lexeme strcpy((char *)malloc(yyleng+1),yytext)

	int lexZeile=1;
	
	
	
%}

%%

	/*Schluesselwoerter */
end		{LD("#LEX: T_%s\n",yytext);return T_END;}
return		{LD("#LEX: T_%s\n",yytext);return T_RETURN;}
goto 		{LD("#LEX: T_%s\n",yytext);return T_GOTO;}
if 		{LD("#LEX: T_%s\n",yytext);return T_IF;}
then 		{LD("#LEX: T_%s\n",yytext);return T_THEN;}
var 		{LD("#LEX: T_%s\n",yytext);return T_VAR;}
not 		{LD("#LEX: T_%s\n",yytext);return T_NOT;}
and 		{LD("#LEX: T_%s\n",yytext);return T_AND;}
"=<" 		{LD("#LEX: T_%s\n",yytext);return T_EQ_LESS;}

	/*Sonderzeichen */
";" 		{LD("#LEX: ; \n"); return ';';}	
"(" 		{LD("#LEX: ( \n"); return '(';}	
")" 		{LD("#LEX: ) \n"); return ')';}	
"," 		{LD("#LEX: , \n"); return ',';}	
":" 		{LD("#LEX: ; \n"); return ':';}	
"=" 		{LD("#LEX: = \n"); return '=';}	
"*" 		{LD("#LEX: * \n"); return '*';}	
"-" 		{LD("#LEX: - \n"); return '-';}	
"+" 		{LD("#LEX: + \n"); return '+';}	
"#" 		{LD("#LEX: # \n"); return '#';}

	/*Identifier*/
({buch}|_)({buch}|{digit}|_)*	{
					LD("#LEX: ident: %s\n",yytext);
					return T_IDENT;	@{@T_IDENT.name@=lexeme; @}
				}	
	/*Numbers*/
&{digit}+	{
			long res=atol(yytext+1);
			LD("#LEX: num %ld\n",res);
			return T_NUM; @{@T_NUM.value@=res; @}
		}
{digit}{hex}*	{
			long res;
			sscanf(yytext, "%lx", &res);	
			LD("#LEX: num %ld\n",res);
			return T_NUM; @{@T_NUM.value@=res; @}
		}

	/*Kommentare*/
\(\*([^*]|\*+[^*)])*\*+\)	{
			LD("#LEX: Commend: %S \n",yytext);
		}

	/*Leerzeichen, Tabs und neue Zeilen Ignorieren*/

[\n\r]		lexZeile++;
[ \t]		;

	/*Alles andere Error*/
.		{
			printf("#LEX: error %s\n",yytext);
			exit(1);	
		}




