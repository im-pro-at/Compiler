GCCFLAG= 

all:
	ox yacc.y lex.l 
	yacc -o yacc.c  oxout.y -d -v 
	gcc -c yacc.c $(GCCFLAG)
	flex -o lex.c oxout.l
	gcc -c lex.c $(GCCFLAG)
	nawk -f bfe.awk burm.bfe | iburg -I  >burm.c
	gcc -c burm.c -Wall $(GCCFLAG) 
	gcc -o gesamt yacc.o lex.o burm.o tools.c list.c vlist.c tree.c reg.c -lfl -Wall $(GCCFLAG) 
	
clean:
	rm -f gesamt oxout.l oxout.y yacc.c yacc.h yacc.output lex.c yacc.o lex.o burm.c burm.o


#DEBUG Tools:


debug:  
	make "GCCFLAG= -Werror -DDEBUG" all
	make debugtest

ldebug:
	make "GCCFLAG= -Werror -DLDEBUG" all
	make debugtest

debugtest:  
	cat test.txt
	./gesamt <test.txt

test:
	/usr/ftp/pub/ubvl/test/gesamt/test >test.output
	tail --lines=6 test.output

%.test: 
	make "GCCFLAG= -g -Werror -DDEBUG" all
	echo -------------INPUT--------------
	cat ../../test/gesamt/$*.0
	echo ------------COMPILE------------
	gesamt <../../test/gesamt/$*.0 >$@.s
	cat $@.s
	echo ------------LINK------------
	sed 's/RET/#include <stdio.h> \nvoid print(int i){printf("RETURN %d",i);} \nmain(){ print/g' ../../test/gesamt/$*.call >$@.c
	echo "return 0;}" >>$@.c
	gcc -o $@ $@.c $@.s -g 
	echo ------------EXE---------------
	cat ../../test/gesamt/$*.call
	./$@
	echo !
 
proper: clean
	rm -f test.output *.test*


