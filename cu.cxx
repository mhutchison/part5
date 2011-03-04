//****************************************************************************
// FILE: cu.cxx 
// Written by: Michael Main
// Complete compiler for the CU Language
// Version date: Feb 3, 2011
// This program must be compiled with the lexer, parser, traverser
// and code generator. It also needs the colorado::tree class.
// The necessary commands are:
// 1. flex -t cu.lex >cu.lex.c
// 2. g++ -Wall -c cu.lex.c
// 3. bison -b cu -d -v cu.y
// 4. g++ -Wall -c cu.y.c
// 5. g++ -Wall -c cu.traverser.cxx
// 6. g++ -Wall -c cu.codegen.cxx
// 7. g++ -Wall -c cu.cxx
// 8. g++ -Wall -c tree.cxx
// 9. g++ cu.o cu.y.o cu.lex.o cu.traverser.o cu.codegen.o tree.o -o cu
// After compilation, you can create a file called sample.cu that
// contains a program written in the CU programming language.
// You can then run cu on that file with the command:
// cu < sample.cu
//*****************************************************************************
#include <iostream>         // Provides cin and cout
#include "cu.tab.h"         // Provides definitions of the token numbers
#include "tree.h"           // Provides the colorado::tree class
using namespace std;        // cout and endl are in std::
using namespace colorado;   // tree class
int yyparse( );             // Provided by the parser
void traverse( );           // Provided by cu.traverser.cxx
void codegen(const tree* p);// The code generator
extern tree* parse_tree_root_ptr; // From the parser

int main( )
{
    cerr << "Starting parsing..." << endl;

    if (yyparse( ) != 0)
	cerr << "Parsing failed." << endl;
    else
    {
	cerr << "Parsing OK." << endl;
	cerr << "Starting traversal..." << endl;
	traverse( );
	if (parse_tree_root_ptr->attribute<int>("Errors") != 0)
	    cerr << "Errors in the traversal." << endl;
	else
	{
	    cerr << "Traversal OK." << endl;
	    cerr << "Starting code generation..." << endl;
	    codegen(parse_tree_root_ptr);
	    cerr << "Done." << endl;
	}
    }

    return 0;
}
