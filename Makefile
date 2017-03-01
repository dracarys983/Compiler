# Makefile

OBJS	= bison.o lex.o main.o ast.o

CC	= g++
CFLAGS	= -g `llvm-config --cxxflags` -std=c++11
LDFLAGS = `llvm-config --ldflags`
LIBS = `llvm-config --libs`

decaf:		$(OBJS)
		$(CC) $(CFLAGS) $(OBJS) $(LDFLSGS) -lpthread $(LIBS) -o decaf -ldl

lex.o:		lex.c
		$(CC) $(CFLAGS) -c lex.c -o lex.o

lex.c:		decaf.l ast.h
		flex decaf.l
		cp lex.yy.c lex.c

bison.o:	bison.c
		$(CC) $(CFLAGS) -c bison.c -o bison.o

bison.c:	decaf.y ast.h
		bison -d -v decaf.y
		cp decaf.tab.c bison.c
		cmp -s decaf.tab.h tok.h || cp decaf.tab.h tok.h

ast.o:		ast.cpp ast.h stdllvm.h
		$(CC) $(CFLAGS) -c ast.cpp -o ast.o

main.o:		main.cpp
		$(CC) $(CFLAGS) -c main.cpp -o main.o

lex.o yac.o main.o	: head.h ast.h
lex.o main.o		: tok.h ast.h

clean:
	rm -f *.o *~ lex.c lex.yy.c bison.c tok.h decaf.tab.c decaf.tab.h decaf.output decaf
