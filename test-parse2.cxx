//****************************************************************************
// FILE: test-parse2.cxx 
// Written by: Michael Main
// Simple test of the parser for the CU Language
// Version date: Jan 31, 2011.
// This program must be compiled with the lexer, parser and traverser.
// It also needs the colorado::tree class.
// The necessary commands are:
// 1. flex -t cu.lex >cu.lex.c
// 2. g++ -Wall -c cu.lex.c
// 3. bison -b cu -d -v cu.y
// 4. g++ -Wall -c cu.y.c
// 5. g++ -Wall -c cu.traverser.cxx
// 6. g++ -Wall -c test-parse2.cxx
// 7. g++ -Wall -c tree.cxx
// 8. g++ test-parse2.o cu.y.o cu.lex.o cu.traverser.o tree.o -o test-parse2
// After compilation, you can create a file called sample.3155 that
// contains a program written in the CSCI 3155 programming language.
// You can then run this test-parse1 on that
// file with the command:
// test-parse2 < sample.cu
//*****************************************************************************
#include <iostream>         // Provides cin and cout
#include "cu.tab.h"         // Provides definitions of the token numbers
#include "tree.h"           // Provides the colorado::tree class
using namespace std;        // cout and endl are in std::
using namespace colorado;   // tree class
int yyparse( );             // Provided by the parser
void traverse( );           // Provided by cu.traverser.cxx
extern tree* parse_tree_root_ptr; // From the parser
#ifndef FULLTREE
#define FULLTREE false      // Print tree's attributes?
#endif

int main( )
{
    cout << "Starting parsing..." << endl;

    if (yyparse( ) != 0)
	cout << "Parsing failed." << endl;
    else
    {
	cout << "Parsing OK." << endl;
	traverse( );
	if (parse_tree_root_ptr->attribute<int>("Errors") != 0)
	    cout << "Errors in the traversal." << endl;
	else
	    parse_tree_root_ptr->write(cout, FULLTREE);
    }

    return 0;
}
