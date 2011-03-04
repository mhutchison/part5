/*****************************************************************************
* FILE: cu.y
* Written by: Michael Main
* Parser specification for the CU programming language.
* Version Jan 31, 2011
* Format of this file:
*   1. C++ Prologue: Code that's needed prior to the definition of yyparse( )
*   2. A list of all tokens
*   3. Rules for operator precedence and associativity
*   4. Grammar rules and actions
*   5. Other C++ code that we need. 
*****************************************************************************/



/*---------------------------------------------------------------------------*/
/* 1. C++ Prologue: Code that's needed prior to the definition of yyparse( ) */
%{
#include "tree.h"                  // Provides tree class
#include "cu.enum.h"               // Provides nonterminals and rhsclasses
using namespace colorado;          // For tree, nonterminals and rhs classes
#define YYSTYPE tree*              // Type of semantic values
tree* parse_tree_root_ptr;         // Will point to root of parse tree

// Functions provided by the Lexical Analyzer (cs3155.lex):
int yylex( );                      // Provided by the lexer
void yyerror(const char* message); // Provided by the lexer
extern int yylineno;               // Provided by the lexer

// Function to set Nonterminal and RHS attributes of a node:
void set_node(tree* ptr, lhs nonterminal, rhs rule);
%}
/*---------------------------------------------------------------------------*/



/*--------------------------------------------------------------------------*/
/* 2. A list of all tokens                                                  */
%token AND ARRAY DO EACH ELSE FALSE FI FLOATCAST FOR FREE
%token FUNCTION IF IN INITIALLY IS NEW NOT NULLPTR OD OF OR POINTER READ
%token REF RETURN RETURNS ROUND THEN TO TRUE UNTIL WHILE WRITE
%token LPAREN LSQUARE PLUS MINUS RPAREN RSQUARE PLUSPLUS MINUSMINUS
%token HAT AT PERCENT STAR SLASH LT LE EQEQ EQ LCURLY GT GE NE
%token RCURLY SEMICOLON COMMA
%token INTEGERVALUE FLOATVALUE STRINGVALUE IDENTIFIER TYPENAME
/*--------------------------------------------------------------------------*/



/*--------------------------------------------------------------------------*/
/* 3. Rules for operator precedence and associativity                       */
%nonassoc LOWEST
%left OR
%left AND
%left EQEQ NE
%left LE LT GE GT
%left PLUS MINUS
%left PERCENT STAR SLASH
%nonassoc UNARYLOW
%right HAT
%nonassoc UNARYHIGH
%nonassoc PLUSPLUS MINUSMINUS
%nonassoc LSQUARE LPAREN
%nonassoc HIGHEST
%start program
/*--------------------------------------------------------------------------*/



/*--------------------------------------------------------------------------*/
/* 4. Grammar rules and actions                                             */
%%
program       : defnlist
	      {
                  $$ = new tree("<program>", 1, $1);
                  set_node($$, program__, __defnlist);
                  parse_tree_root_ptr = $$;
	      }
              ;

body          : LCURLY defnlist stmtlist RCURLY 
              {
                  $$ = new tree("<body>", 4, $1, $2, $3, $4);
                  set_node($$, body__, __LCURLY_defnlist_stmtlist_RCURLY);
              }
              ;

defn          : vardefn
              {
                  $$ = new tree("<defn>", 1, $1);
                  set_node($$, defn__, __vardefn);
              }
	      | funcdefn
              {
                  $$ = new tree("<defn>", 1, $1);
                  set_node($$, defn__, __funcdefn);
              }
              ;

defnlist      : /* EMPTY */
              {
                  $$ = new tree("<defnlist>");
                  set_node($$, defnlist__, __EMPTY);
              }
              | defnlist defn
              {
                  $$ = new tree("<defnlist>", 2, $1, $2);
                  set_node($$, defnlist__, __defnlist_defn);
              }
              ;

expr          : INTEGERVALUE
              {
                  $$ = new tree("<expr>", 1, $1);
                  set_node($$, expr__, __INTEGERVALUE);
              }
	      | FLOATVALUE
              {
                  $$ = new tree("<expr>", 1, $1);
                  set_node($$, expr__, __FLOATVALUE);
              }
              | STRINGVALUE
              {
                  $$ = new tree("<expr>", 1, $1);
                  set_node($$, expr__, __STRINGVALUE);
              }
              | TRUE
              {
                  $$ = new tree("<expr>", 1, $1);
                  set_node($$, expr__, __TRUE);
              }
              | FALSE
              {
                  $$ = new tree("<expr>", 1, $1);
                  set_node($$, expr__, __FALSE);
              }
              | NULLPTR
              {
                  $$ = new tree("<expr>", 1, $1);
                  set_node($$, expr__, __NULLPTR);
              }
              | IDENTIFIER
              {
                  $$ = new tree("<expr>", 1, $1);
                  set_node($$, expr__, __IDENTIFIER);
              }
              | LPAREN ARRAY OF typeexpr IS exprseq RPAREN
              {
                  $$ = new tree("<expr>", 7, $1, $2, $3, $4, $5, $6, $7);
                  set_node($$, expr__, __LPAREN_ARRAY_OF_typeexpr_IS_exprseq_RPAREN);
              }
              | LPAREN NEW typeexpr IS expr RPAREN
              {
                  $$ = new tree("<expr>", 6, $1, $2, $3, $4, $5, $6);
                  set_node($$, expr__, __LPAREN_NEW_typeexpr_IS_expr_RPAREN);
              }
              | LPAREN expr RPAREN
              {
                  $$ = new tree("<expr>", 3, $1, $2, $3);
                  set_node($$, expr__, __LPAREN_expr_RPAREN);
              }
              | expr OR expr
              {
                  $$ = new tree("<expr>", 3, $1, $2, $3);
                  set_node($$, expr__, __expr_OR_expr);
              }
              | expr AND expr
              {
                  $$ = new tree("<expr>", 3, $1, $2, $3);
                  set_node($$, expr__, __expr_AND_expr);
              }
              | expr EQEQ expr
              {
                  $$ = new tree("<expr>", 3, $1, $2, $3);
                  set_node($$, expr__, __expr_EQEQ_expr);
              }
              | expr NE expr
              {
                  $$ = new tree("<expr>", 3, $1, $2, $3);
                  set_node($$, expr__, __expr_NE_expr);
              }
              | expr LT expr
              {
                  $$ = new tree("<expr>", 3, $1, $2, $3);
                  set_node($$, expr__, __expr_LT_expr);
              }
              | expr GT expr
              {
                  $$ = new tree("<expr>", 3, $1, $2, $3);
                  set_node($$, expr__, __expr_GT_expr);
              }
              | expr LE expr
              {
                  $$ = new tree("<expr>", 3, $1, $2, $3);
                  set_node($$, expr__, __expr_LE_expr);
              }
              | expr GE expr
              {
                  $$ = new tree("<expr>", 3, $1, $2, $3);
                  set_node($$, expr__, __expr_GE_expr);
              }
              | expr PLUS expr
              {
                  $$ = new tree("<expr>", 3, $1, $2, $3);
                  set_node($$, expr__, __expr_PLUS_expr);
              }
              | expr MINUS expr
              {
                  $$ = new tree("<expr>", 3, $1, $2, $3);
                  set_node($$, expr__, __expr_MINUS_expr);
              }
              | expr STAR expr
              {
                  $$ = new tree("<expr>", 3, $1, $2, $3);
                  set_node($$, expr__, __expr_STAR_expr);
              }
              | expr SLASH expr
              {
                  $$ = new tree("<expr>", 3, $1, $2, $3);
                  set_node($$, expr__, __expr_SLASH_expr);
              }
              | expr PERCENT expr
              {
                  $$ = new tree("<expr>", 3, $1, $2, $3);
                  set_node($$, expr__, __expr_PERCENT_expr);
              }
              | MINUS expr %prec UNARYLOW
              {
                  $$ = new tree("<expr>", 2, $1, $2);
                  set_node($$, expr__, __MINUS_expr);
              }
              | PLUS expr %prec UNARYLOW
              {
                  $$ = new tree("<expr>", 2, $1, $2);
                  set_node($$, expr__, __PLUS_expr);
              }
              | NOT expr %prec UNARYLOW
              {
                  $$ = new tree("<expr>", 2, $1, $2);
                  set_node($$, expr__, __NOT_expr);
              }
              | AT expr %prec UNARYHIGH
              {
                  $$ = new tree("<expr>", 2, $1, $2);
                  set_node($$, expr__, __AT_expr);
              }             
              | STAR expr %prec UNARYHIGH
              {
                  $$ = new tree("<expr>", 2, $1, $2);
                  set_node($$, expr__, __STAR_expr);
              }
              | ROUND expr %prec UNARYHIGH
              {
                  $$ = new tree("<expr>", 2, $1, $2);
                  set_node($$, expr__, __ROUND_expr);
              }
              | FLOATCAST expr %prec UNARYHIGH
              {
                  $$ = new tree("<expr>", 2, $1, $2);
                  set_node($$, expr__, __FLOATCAST_expr);
              }
              | expr HAT expr
              {
                  $$ = new tree("<expr>", 3, $1, $2, $3);
                  set_node($$, expr__, __expr_HAT_expr);
              }
              | expr LSQUARE expr RSQUARE
              {
                  $$ = new tree("<expr>", 4, $1, $2, $3, $4);
                  set_node($$, expr__, __expr_LSQUARE_expr_RSQUARE);
              }
              | PLUSPLUS expr %prec UNARYHIGH
              {
                  $$ = new tree("<expr>", 2, $1, $2);
                  set_node($$, expr__, __PLUSPLUS_expr);
              }
              | expr PLUSPLUS
              {
                  $$ = new tree("<expr>", 2, $1, $2);
                  set_node($$, expr__, __expr_PLUSPLUS);
              }
              | MINUSMINUS expr %prec UNARYHIGH
              {
                  $$ = new tree("<expr>", 2, $1, $2);
                  set_node($$, expr__, __MINUSMINUS_expr);
              }
              | expr MINUSMINUS
              {
                  $$ = new tree("<expr>", 2, $1, $2);
                  set_node($$, expr__, __expr_MINUSMINUS);
              }
              | IDENTIFIER LPAREN exprseq RPAREN
              {
                  $$ = new tree("<expr>", 4, $1, $2, $3, $4);
                  set_node($$, expr__, __IDENTIFIER_LPAREN_exprseq_RPAREN);
              }
              ;

exprseq       : /* EMPTY */
              {
                  $$ = new tree("<exprseq>");
                  set_node($$, exprseq__, __EMPTY);
              }
              | expr
              {
                  $$ = new tree("<exprseq>", 1, $1);
                  set_node($$, exprseq__, __expr);
              }
              | exprseq COMMA expr
              {
                  $$ = new tree("<exprseq>", 3, $1, $2, $3);
                  set_node($$, exprseq__, __exprseq_COMMA_expr);
              }
              ;

funcdefn      : FUNCTION IDENTIFIER LPAREN parmseq RPAREN body 
              {
                  $$ = new tree("<funcdefn>", 6, $1, $2, $3, $4, $5, $6);
                  set_node($$, funcdefn__, __FUNCTION_IDENTIFIER_LPAREN_parmseq_RPAREN_body);
              }
              | FUNCTION IDENTIFIER LPAREN parmseq RPAREN RETURNS typeexpr body
              {
                  $$ = new tree("<funcdefn>", 8, $1, $2, $3, $4, $5, $6, $7, $8);
                  set_node($$, funcdefn__, __FUNCTION_IDENTIFIER_LPAREN_parmseq_RPAREN_RETURNS_typeexpr_body);
              }
              ;

parmdefn      : typeexpr IDENTIFIER
              {
                  $$ = new tree("<parmdefn>", 2, $1, $2);
                  set_node($$, parmdefn__, __typeexpr_IDENTIFIER);
              }
              | REF typeexpr IDENTIFIER
              {
                  $$ = new tree("<parmdefn>", 3, $1, $2, $3);
                  set_node($$, parmdefn__, __REF_typeexpr_IDENTIFIER);
              }
              ;

parmseq       : /* EMPTY */
              {
                  $$ = new tree("<parmseq>");
                  set_node($$, parmseq__, __EMPTY);
              }
              | parmdefn
              {
                  $$ = new tree("<parmseq>", 1, $1);
                  set_node($$, parmseq__, __parmdefn);
              }
              | parmseq COMMA parmdefn
              {
                  $$ = new tree("<parmseq>", 3, $1, $2, $3);
                  set_node($$, parmseq__, __parmseq_COMMA_parmdefn);
              }
	      ;

stmt          : SEMICOLON
              {
                  $$ = new tree("<stmt>", 1, $1);
                  set_node($$, stmt__, __SEMICOLON);
              }
              | READ expr SEMICOLON
              {
                  $$ = new tree("<stmt>", 3, $1, $2, $3);
                  set_node($$, stmt__, __READ_expr_SEMICOLON);
              }
              | WRITE expr SEMICOLON
              {
                  $$ = new tree("<stmt>", 3, $1, $2, $3);
                  set_node($$, stmt__, __WRITE_expr_SEMICOLON);
              }
              | expr EQ expr SEMICOLON
              {
                  $$ = new tree("<stmt>", 4, $1, $2, $3, $4);
                  set_node($$, stmt__, __expr_EQ_expr_SEMICOLON);
              }
              | IF expr THEN stmt FI
              {
                  $$ = new tree("<stmt>", 5, $1, $2, $3, $4, $5);
                  set_node($$, stmt__, __IF_expr_THEN_stmt_FI);
              }
              | IF expr THEN stmt ELSE stmt FI
              {
                  $$ = new tree("<stmt>", 7, $1, $2, $3, $4, $5, $6, $7);
                  set_node($$, stmt__, __IF_expr_THEN_stmt_ELSE_stmt_FI);
              }
              | WHILE expr DO stmt OD
              {
                  $$ = new tree("<stmt>", 5, $1, $2, $3, $4, $5);
                  set_node($$, stmt__, __WHILE_expr_DO_stmt_OD);
              }
              | DO stmt UNTIL expr OD
              {
                  $$ = new tree("<stmt>", 5, $1, $2, $3, $4, $5);
                  set_node($$, stmt__, __DO_stmt_UNTIL_expr_OD);
              }
              | FOR EACH IDENTIFIER IN expr DO stmt OD
              {
                  $$ = new tree("<stmt>", 8, $1, $2, $3, $4, $5, $6, $7, $8);
                  set_node($$, stmt__, __FOR_EACH_IDENTIFIER_IN_expr_DO_stmt_OD);
              }
              | LCURLY stmtlist RCURLY
              {
                  $$ = new tree("<stmt>", 3, $1, $2, $3);
                  set_node($$, stmt__, __LCURLY_stmtlist_RCURLY);
              }
              | RETURN SEMICOLON
              {
                  $$ = new tree("<stmt>", 2, $1, $2);
                  set_node($$, stmt__, __RETURN_SEMICOLON);
              }
              | RETURN expr SEMICOLON
              {
                  $$ = new tree("<stmt>", 3, $1, $2, $3);
                  set_node($$, stmt__, __RETURN_expr_SEMICOLON);
              }
              | FREE expr SEMICOLON
              {
                  $$ = new tree("<stmt>", 3, $1, $2, $3);
                  set_node($$, stmt__, __FREE_expr_SEMICOLON);
              }
              | IDENTIFIER LPAREN exprseq RPAREN SEMICOLON
              {
                  $$ = new tree("<stmt>", 5, $1, $2, $3, $4, $5);
                  set_node($$, stmt__, __IDENTIFIER_LPAREN_exprseq_RPAREN_SEMICOLON);
              }
              ;

stmtlist      : /* EMPTY */
              {
                  $$ = new tree("<stmtlist>");
                  set_node($$, stmtlist__, __EMPTY);
              }
              | stmtlist stmt
              {
                  $$ = new tree("<stmtlist>", 2, $1, $2);
                  set_node($$, stmtlist__, __stmtlist_stmt);
              }
              ;

typeexpr      : TYPENAME
              {
                  $$ = new tree("<typeexpr>", 1, $1);
                  set_node($$, typeexpr__, __TYPENAME);
              }
              | ARRAY OF typeexpr
              {
                  $$ = new tree("<typeexpr>", 3, $1, $2, $3);
                  set_node($$, typeexpr__, __ARRAY_OF_typeexpr);
              }
              | POINTER TO typeexpr
              {
                  $$ = new tree("<typeexpr>", 3, $1, $2, $3);
                  set_node($$, typeexpr__, __POINTER_TO_typeexpr);
              }
              ;

vardefn       : typeexpr IDENTIFIER SEMICOLON
              {
                  $$ = new tree("<vardefn>", 3, $1, $2, $3);
                  set_node($$, vardefn__, __typeexpr_IDENTIFIER_SEMICOLON);
              }
              | typeexpr IDENTIFIER IS INITIALLY expr SEMICOLON
              {
                  $$ = new tree("<vardefn>", 6, $1, $2, $3, $4, $5, $6);
                  set_node($$, vardefn__, __typeexpr_IDENTIFIER_IS_INITIALLY_expr_SEMICOLON);
              }
              ;
%%
/*--------------------------------------------------------------------------*/



/*--------------------------------------------------------------------------*/
/* 5. Other C++ code that we need.                                          */
void set_node(tree* p, lhs nonterminal, rhs rule)
{
    p->set_attribute<lhs>("LHS", nonterminal);
    p->set_attribute<rhs>("RHS", rule);
    p->set_attribute<int>("Errors", 0);

    if (p->many_children( ) > 0)
        p->set_attribute<int>("Line", p->child(0)->attribute<int>("Line"));
    else
        p->set_attribute<int>("Line", yylineno);
}
/*--------------------------------------------------------------------------*/
