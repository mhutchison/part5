/*****************************************************************************
* FILE: cu.lex 
* This flex file generates a lexical analyzer for the CSCI 3155 Language.
* Written by: Michael Main (main@colorado.edu)
* Version date: Jan 31, 2011 (complete solution)
* Format of this file:
*   1. C++ Prologue: Code that's needed prior to the definition of yylex( ) 
*   2. Definitions of useful regular expressions
*   3. Flex options.
*   4. Pattern/action rules that flex uses to create the yylex( ) function.
*   5. Epilogue: Other C++ code that we need.
*****************************************************************************/



/*---------------------------------------------------------------------------*/
/* 1. C++ Prologue: Code that's needed prior to the definition of yylex( )   */
%{
#include "tree.h"                 // Provides simple tree class
#define YYSTYPE colorado::tree*   // Define data type attached to tokens
#include "cu.tab.h"               // Provides token numbers
void one_node(int token_number);  // Sets yylval to point to a one-node tree
%}
/*---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*/
/* 2. Definitions of useful regular expressions.                             */
comment1                      [\/][\/].*
comment2                      [\/][\*]([^\*]|[\*]+[^\*\/])*[\*]*[\/]?
digits                        [0-9]+
float1                        [0-9]+[\.][0-9]*([eE][\+\-]?[0-9]+)?
float2                        [0-9]*[\.][0-9]+([eE][\+\-]?[0-9]+)?
float3                        [0-9]+([eE][\+\-]?[0-9]+)?
name                          [a-zA-Z\_][a-zA-Z\_0-9]*
separator                     [ \r\t\v\f\n]
str                           [\"]([^\"\\\n]|([\\](.|[\n])))*[\"]
/*---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*/
/* 3. Flex options.                                                          */
/* The noyywrap option forces  yylex( ) to stop at the end of the first      */
/* input file.  The yylineno option directs yylex to maintain a global       */
/* int yylineno that contains the current input line number.                 */
%option noyywrap              
%option yylineno
/*---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*/
/* 4. Pattern/action rules that flex uses to create the yylex( ) function.   */
%%
and                one_node(AND); return AND;
array              one_node(ARRAY); return ARRAY;
do                 one_node(DO); return DO;
each               one_node(EACH); return EACH;
else               one_node(ELSE); return ELSE;
false              one_node(FALSE); return FALSE;
fi                 one_node(FI); return FI;
floatcast          one_node(FLOATCAST); return FLOATCAST;
for                one_node(FOR); return FOR;
free               one_node(FREE); return FREE;
function           one_node(FUNCTION); return FUNCTION;
if                 one_node(IF); return IF;
in                 one_node(IN); return IN;
initially          one_node(INITIALLY); return INITIALLY;
is                 one_node(IS); return IS;
new                one_node(NEW); return NEW;
not                one_node(NOT); return NOT;
nullptr            one_node(NULLPTR); return NULLPTR;
od                 one_node(OD); return OD;
of                 one_node(OF); return OF;
or                 one_node(OR); return OR;
pointer            one_node(POINTER); return POINTER;
read               one_node(READ); return READ;
ref                one_node(REF); return REF;
return             one_node(RETURN); return RETURN;
returns            one_node(RETURNS); return RETURNS;
round              one_node(ROUND); return ROUND;
then               one_node(THEN); return THEN;
to                 one_node(TO); return TO;
true               one_node(TRUE); return TRUE;
until              one_node(UNTIL); return UNTIL;
while              one_node(WHILE); return WHILE;
write              one_node(WRITE); return WRITE;

[\(]               one_node(LPAREN); return LPAREN;
[\[]               one_node(LSQUARE); return LSQUARE;
[\+]               one_node(PLUS); return PLUS;
[\-]               one_node(MINUS); return MINUS;
[\)]               one_node(RPAREN); return RPAREN;
[\]]               one_node(RSQUARE); return RSQUARE;
[\+][\+]           one_node(PLUSPLUS); return PLUSPLUS;
[\-][\-]           one_node(MINUSMINUS); return MINUSMINUS;
[\^]               one_node(HAT); return HAT;
[\@]               one_node(AT); return AT;
[\%]               one_node(PERCENT); return PERCENT;
[\*]               one_node(STAR); return STAR;
[\/]               one_node(SLASH); return SLASH;
[\<]               one_node(LT); return LT;
[\<][\=]           one_node(LE); return LE;
[\=][\=]           one_node(EQEQ); return EQEQ;
[\=]               one_node(EQ); return EQ;
[\{]               one_node(LCURLY); return LCURLY;
[\>]               one_node(GT); return GT;
[\>][\=]           one_node(GE); return GE;
[\!][\=]           one_node(NE); return NE;
[\}]               one_node(RCURLY); return RCURLY;
[\;]               one_node(SEMICOLON); return SEMICOLON;
[\,]               one_node(COMMA); return COMMA;

{name}             one_node(IDENTIFIER); return IDENTIFIER;
[\|][^\|\n]*[\|]   one_node(TYPENAME); return TYPENAME; 
{str}              one_node(STRINGVALUE); return STRINGVALUE;
{digits}           one_node(INTEGERVALUE); return INTEGERVALUE;
{float1}           one_node(FLOATVALUE); return FLOATVALUE;
{float2}           one_node(FLOATVALUE); return FLOATVALUE;
{float3}           one_node(FLOATVALUE); return FLOATVALUE;

<<EOF>>            return 0;
{comment1}         ; // No action
{comment2}         ; // No action
{separator}        ; // No action
.                  return yytext[0];
%%
/*---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*/
/* 5. Other C++ code that we need.                                           */
#include <iostream>     // Provides std::cerr and std::endl
#include <string>       // Provides string class
using namespace std;    // Needed for string, cerr, and endl
void yyerror(const char* message)
{
    cerr << "ERROR at token " << '\"' << yytext << '\"' << ':' << endl;
    cerr << message << endl;
    cerr << "Line: " << yylineno << endl;
}
// yylex( ) calls one_node to set yylval before returning a token.
void one_node(int token_number)
{
    colorado::tree* answer = new colorado::tree(string(yytext));
    answer->set_attribute<int>("Token", token_number);
    answer->set_attribute<int>("Line", yylineno);
    answer->set_attribute<int>("Errors", 0);
    yylval = answer;
}
/*---------------------------------------------------------------------------*/
