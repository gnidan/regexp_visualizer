## makefile for CS360, Assignment 2
#

cc=gcc -g

.PHONY : byHand byGen clean

byHand : byhand.out
	byhand.out

byhand.out : main.c dot_output.c dfsm.c fsm.c dfsm.h dot_output.h fsm.h 
	$(cc) -o byhand.out main.c fsm.c dot_output.c dfsm.c

byGen : bygen.out
	bygen.out

bygen.out : dfsm.c dot_output.c fsm.c lex.yy.c regexp.tab.c dfsm.h dot_output.h fsm.h 
	$(cc) -o bygen.out regexp.tab.c lex.yy.c fsm.c dot_output.c dfsm.c -ly -lfl

lex.yy.c : regexp.tab.c regexp.tab.h
	flex regexp.l

regexp.tab.c : regexp.tab.h
regexp.tab.h :
	bison -d regexp.y

clean :
	-\rm *.out
	-\rm *.dot
	-\rm lex.yy.c
	-\rm regexp.tab.?


