//****************************************************************************
// FILE: test-parse1.cxx 
// Written by: Michael Main
// Simple test of the parser for the CSCI 3155 Language
// Version date: Jan 8, 2005.
// This program must be compiled with the lexer and parser
// that flex and bison produce from cs3155.lex and cs3155.y.
// The necessary commands are:
// 1. flex -t cs3155.lex >cs3155.lex.c
// 2. g++ -Wall -c cs3155.lex.c
// 3. bison -b cs3155 -d -v cs3155.y
// 4. g++ -Wall -c cs3155.y.c
// 5. g++ -Wall -c test-parse1.cxx
// 6. g++ test-parse1.o cs3155.y.o cs3155.lex.o -o test-parse1
// After compilation, you can create a file called sample.3155 that
// contains a program written in the CSCI 3155 programming language.
// You can then run this test-parse1 on that
// file with the command:
// test-parse1 < sample.3155
//*****************************************************************************
#include <iostream>         // Provides cin and cout
using namespace std;        // cout and endl are in std::
int yyparse( );             // Provided by the parser

int main( )
{
    cout << "Starting parsing..." << endl;

    if (yyparse( ) != 0)
	cout << "Parsing failed." << endl;
    else
    {
	cout << "Parsing OK." << endl;
    }

    return 0;
}
