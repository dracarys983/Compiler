# Makefile

LLVM_CONFIG="/usr/local/bin/llvm-config"
OBJS	= bison.o lex.o main.o ast.o

CC	= g++
CFLAGS	= -g -w `$(LLVM_CONFIG) --cxxflags` -std=c++11
LDFLAGS = `$(LLVM_CONFIG) --ldflags`
LIBS = `$(LLVM_CONFIG) --libs`

decaf:		$(OBJS)
		$(CC) $(CFLAGS) $(OBJS) $(LDFLSGS) -fPIC -lpthread $(LIBS) -ltinfo -o decaf -ldl
		mkdir gen && mv *.o lex.c lex.yy.c bison.c tok.h decaf.tab.c decaf.tab.h decaf.output gen/

lex.o:		lex.c
		$(CC) $(CFLAGS) -c lex.c -o lex.o

lex.c:		decaf.l include/ast.h
		flex decaf.l
		cp lex.yy.c lex.c

bison.o:	bison.c
		$(CC) $(CFLAGS) -c bison.c -o bison.o

bison.c:	decaf.y include/ast.h
		bison -d -v decaf.y
		cp decaf.tab.c bison.c
		cmp -s decaf.tab.h tok.h || cp decaf.tab.h tok.h

ast.o:		ast.cpp include/ast.h include/stdllvm.h
		$(CC) $(CFLAGS) -c ast.cpp -o ast.o

main.o:		main.cpp
		$(CC) $(CFLAGS) -c main.cpp -o main.o
		

lex.o yac.o main.o	: include/head.h include/ast.h
lex.o main.o		: tok.h include/ast.h

clean:
	rm -rf gen decaf
