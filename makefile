###############################################################################
# Makefile for the CSCI 3155 Programming Language Homework Project
# WRITTEN BY: Michael Main (main@colorado.edu), Jan 11, 2011
#
# Define the suffix for an executable file:
SUFFIX =
ifdef ComSpec 
  SUFFIX = .exe
endif
ifdef COMSPEC 
  SUFFIX = .exe
endif
# Local makefile variables
EXPENDABLES = \
    test-lexer test-parse1 test-parse2 test-parse2-full test-traverser \
    compiler \
    *.exe *.o \
    cu.lex.c cu.tab.c \
    cu.output core
BISONFILES = $(wildcard cu.y)
TREEFILES = $(wildcard tree.h)
###############################################################################


###############################################################################
# Rules for Homework Assignment 1: For test-lexer or test-lexer.exe.
hw1:
	@make test-lexer$(SUFFIX)
ifeq ($(TREEFILES),tree.h)
test-lexer$(SUFFIX): test-lexer.o cu.tab.o cu.lex.o tree.o 
	g++ -Wall -gstabs test-lexer.o cu.tab.o cu.lex.o tree.o -o test-lexer
cu.lex.o: cu.lex.c cu.tab.h tree.h
	g++ -gstabs -c cu.lex.c
else
test-lexer$(SUFFIX): test-lexer.o cu.lex.o
	g++ -Wall -gstabs test-lexer.o cu.lex.o -o test-lexer
cu.lex.o: cu.lex.c cu.tab.h
	g++ -gstabs -c cu.lex.c
endif
cu.lex.c: cu.lex
	flex -t cu.lex >cu.lex.c
test-lexer.o: test-lexer.cxx cu.tab.h
	g++ -Wall -gstabs -c test-lexer.cxx
###############################################################################


###############################################################################
# Rules for Homework Assignment 2: For test-parse1 or test-parse1.exe.
hw2:
	@make test-parse1$(SUFFIX)
ifeq ($(TREEFILES),tree.h)
test-parse1$(SUFFIX): test-parse1.o cu.tab.o cu.lex.o tree.o
	g++ -gstabs test-parse1.o cu.tab.o cu.lex.o tree.o -o test-parse1
cu.tab.o: cu.tab.c cu.tab.h cu.enum.h
	g++ -gstabs -c cu.tab.c
else
test-parse1$(SUFFIX): test-parse1.o cu.tab.o cu.lex.o
	g++ -gstabs test-parse1.o cu.tab.o cu.lex.o -o test-parse1
cu.tab.o: cu.tab.c cu.tab.h 
	g++ -gstabs -c cu.tab.c
endif
ifeq ($(BISONFILES),cu.y)
cu.tab.c cu.tab.h: cu.y
	bison -d -b cu -v cu.y
endif
test-parse1.o: test-parse1.cxx cu.tab.h
	g++ -Wall -gstabs -c test-parse1.cxx
###############################################################################


###############################################################################
# Rules for Homework Assignment 3 or 4: For test-parse2 or test-parse2.exe,
# and test-parse2-full or test-parse2-full.exe
hw3 hw4:
	@make test-parse2$(SUFFIX) test-parse2-full$(SUFFIX)
test-parse2$(SUFFIX): test-parse2.o cu.tab.o cu.lex.o cu.traverser.o tree.o
	g++ -gstabs test-parse2.o cu.tab.o cu.lex.o cu.traverser.o tree.o -o test-parse2
test-parse2.o: test-parse2.cxx cu.tab.h tree.h 
	g++ -Wall -gstabs -c test-parse2.cxx
test-parse2-full$(SUFFIX): test-parse2-full.o cu.tab.o cu.lex.o cu.traverser.o tree.o
	g++ -gstabs test-parse2-full.o cu.tab.o cu.lex.o cu.traverser.o tree.o -o test-parse2-full
test-parse2-full.o: test-parse2.cxx cu.tab.h tree.h 
	g++ -Wall -gstabs -c -DFULLTREE=true test-parse2.cxx -o test-parse2-full.o
cu.traverser.o: cu.traverser.cxx tree.h symtab.h symtab.template cu.tab.h cu.enum.h
	g++ -Wall -gstabs -c cu.traverser.cxx 
tree.o: tree.cxx tree.h
	g++ -Wall -gstabs -c tree.cxx
###############################################################################


###############################################################################
# Rules for Homework Assignment 5-7: For cu or cu.exe
hw5 hw6 hw7:
	@make cu$(SUFFIX)
cu$(SUFFIX): cu.o cu.tab.o cu.lex.o cu.traverser.o cu.codegen.o tree.o
	g++ -gstabs cu.o cu.tab.o cu.lex.o cu.traverser.o cu.codegen.o tree.o -o cu
cu.o: cu.cxx cu.tab.h tree.h 
	g++ -Wall -gstabs -c cu.cxx
cu.codegen.o: cu.codegen.cxx tree.h cu.tab.h cu.enum.h
	g++ -Wall -gstabs -c cu.codegen.cxx 
###############################################################################


###############################################################################
# Artificial rules, including two empty commands to prevent cu.y
# and cu.lex from being picked up by implicit rules.
cu.y: ;
cu.lex: ;
clean:
	-rm -f $(EXPENDABLES)
all:
	-rm -f $(EXPENDABLES)
	@make test-lexer$(SUFFIX) test-parse1$(SUFFIX)
###############################################################################

