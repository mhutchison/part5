//****************************************************************************
// FILE: test-lexer.cxx 
// Written by: Michael Main
// Lexer for the CSCI 3155 Language
// Version date: Sep 14, 2006
// This program must be compiled with the lexer that flex produces from
// cs3155.lex.  The necessary commands are:
// 1. flex -t cs3155.lex >cs3155.lex.c
// 2. g++ -Wall -c cs3155.lex.c
// 3. g++ -Wall -c test-lexer.cxx
// 4. g++ -Wall -c tree.cxx
// 4. g++ test-lexer.o cs3155.lex.o tree.o -o test-lexer
// After compilation, you can create a file called tokens.txt that
// contains one or more instances of each kind of token that you
// want to recognize.  You can then run this test-lexer on that
// file with the command:
// test-lexer <tokens.txt
//*****************************************************************************
#include <iostream>        // Provides cin and cout
#include "cu.tab.h"        // Defines the token numbers
using namespace std;       // cout and endl are in std::
int yylex( );              // Provided by flex
extern char* yytext;       // Provided flex
const int END_OF_FILE = 0; // Token number for the end of file

int main( )
{
    int next_token;
    while ((next_token = yylex( )) != END_OF_FILE)
    {
	switch (next_token)
	{
	case AND: cout << "TOKEN: AND"; break;
	case ARRAY: cout << "TOKEN: ARRAY"; break;
	case CONSTANT: cout << "TOKEN: CONSTANT"; break;
	case DO: cout << "TOKEN: DO"; break;
	case EACH: cout << "TOKEN: EACH"; break;
	case ELSE: cout << "TOKEN: ELSE"; break;
	case FALSE: cout << "TOKEN: FALSE"; break;
	case FI: cout << "TOKEN: FI"; break;
	case FOR: cout << "TOKEN: FOR"; break;
	case FREE: cout << "TOKEN: FREE"; break;
	case FUNCTION: cout << "TOKEN: FUNCTION"; break;
	case IF: cout << "TOKEN: IF"; break;
	case IN: cout << "TOKEN: IN"; break;
	case NEW: cout << "TOKEN: NEW"; break;
	case NOT: cout << "TOKEN: NOT"; break;
	case OD: cout << "TOKEN: OD"; break;
	case OF: cout << "TOKEN: OF"; break;
	case OR: cout << "TOKEN: OR"; break;
	case POINTER: cout << "TOKEN: POINTER"; break;
	case READ: cout << "TOKEN: READ"; break;
	case RETURN: cout << "TOKEN: RETURN"; break;
	case RETURNS: cout << "TOKEN: RETURNS"; break;
	case ROUND: cout << "TOKEN: ROUND"; break;
	case THEN: cout << "TOKEN: THEN"; break;
	case TO: cout << "TOKEN: TO"; break;
	case TRUE: cout << "TOKEN: TRUE"; break;
	case UNTIL: cout << "TOKEN: UNTIL"; break;
	case WHILE: cout << "TOKEN: WHILE"; break;
	case WRITE: cout << "TOKEN: WRITE"; break;
	    
	case LPAREN: cout << "TOKEN: LPAREN"; break;
	case LSQUARE: cout << "TOKEN: LSQUARE"; break;
	case PLUS: cout << "TOKEN: PLUS"; break;
	case MINUS: cout << "TOKEN: MINUS"; break;
	case RPAREN: cout << "TOKEN: RPAREN"; break;
	case RSQUARE: cout << "TOKEN: RSQUARE"; break;
	case PLUSPLUS: cout << "TOKEN: PLUSPLUS"; break;
	case MINUSMINUS: cout << "TOKEN: MINUSMINUS"; break;
	case HAT: cout << "TOKEN: HAT"; break;
	case AT: cout << "TOKEN: AT"; break;
	case PERCENT: cout << "TOKEN: PERCENT"; break;
	case STAR: cout << "TOKEN: STAR"; break;
	case SLASH: cout << "TOKEN: SLASH"; break;
	case LT: cout << "TOKEN: LT"; break;
	case LE: cout << "TOKEN: LE"; break;
	case EQEQ: cout << "TOKEN: EQEQ"; break;
	case EQ: cout << "TOKEN: EQ"; break;
	case LCURLY: cout << "TOKEN: LCURLY"; break;
	case GT: cout << "TOKEN: GT"; break;
	case GE: cout << "TOKEN: GE"; break;
	case NE: cout << "TOKEN: NE"; break;
	case RCURLY: cout << "TOKEN: RCURLY"; break;
	case BAR: cout << "TOKEN: BAR"; break;
	case SEMICOLON: cout << "TOKEN: SEMICOLON"; break;
	case COMMA: cout << "TOKEN: COMMA"; break;
	    
	case IDENTIFIER:
	    cout << "IDENTIFIER: " << '\"' << yytext << '\"';
	    break;
	case TYPENAME:
	    cout << "TYPENAME: " << '\"' << yytext << '\"';
	    break;
	case INTEGERVALUE:
	    cout << "INTEGERVALUE: " << '\"' << yytext << '\"'; 
	    break;
	case STRINGVALUE:
	    cout << "STRINGVALUE: " << '\"' << yytext << '\"'; 
	    break;
	case FLOATVALUE:
	    cout << "FLOATVALUE: " << '\"' << yytext << '\"'; 
	    break;
	
	default:
	    if ((next_token > 0) && (next_token <=255))
	    {
		cout << "CHAR " << next_token;
		cout << " (" << static_cast<char>(next_token) << ')';
	    }
	    else
		cout << "UNKNOWN TOKEN " << next_token;
	}
	cout << endl;
    }
    cout << "END OF FILE." << endl;
    return 0;
}
