// File: cu-enum.h
// Written by: Michael Main
// This file provides two enum type definitions.
// The elements of the cs3155::nonterminal enum type are the nonterminals of
// the gramar for the CSCI 3155 Programming Language.  The elements of
// the cs3155::rhs enum type are the right-hand sides of the grammar rules.
// Version Jan 31, 2011
#ifndef CU_GRAMMAR_H
#define CU_GRAMMAR_H

//-------------------------------------------------------------------------
// Definitions of an enum type to describe the nonterminals of our grammar:
enum lhs {
    ILLEGAL_NONTERMINAL,
    program__,
    body__,
    defn__,
    defnlist__,
    expr__,
    exprseq__,
    funcdefn__,
    parmdefn__,
    parmseq__,
    stmt__,
    stmtlist__,
    typeexpr__,
    vardefn__
};
//-------------------------------------------------------------------------

    
    //-------------------------------------------------------------------------
    // Definitions of an enum type to describe the right hand side of a rule:
enum rhs {
    ILLEGAL_RHS,
	
    __EMPTY, // Use this for empty right hand side
	
    // program -->
    __defnlist,
	
    // body -->
    __LCURLY_defnlist_stmtlist_RCURLY,

    // defn -->
    __vardefn,
    __funcdefn,

    // defnlist -->
    // could be empty or...
    __defnlist_defn,

    // expr -->
    __INTEGERVALUE,
    __FLOATVALUE,
    __STRINGVALUE,
    __TRUE,
    __FALSE,
    __NULLPTR,
    __IDENTIFIER,
    __LPAREN_ARRAY_OF_typeexpr_IS_exprseq_RPAREN,
    __LPAREN_NEW_typeexpr_IS_expr_RPAREN,
    __LPAREN_expr_RPAREN,
    __expr_OR_expr,
    __expr_AND_expr,
    __expr_EQEQ_expr,
    __expr_NE_expr,
    __expr_LT_expr,
    __expr_GT_expr,
    __expr_LE_expr,
    __expr_GE_expr,
    __expr_PLUS_expr,
    __expr_MINUS_expr,
    __expr_STAR_expr,
    __expr_SLASH_expr,
    __expr_PERCENT_expr,
    __MINUS_expr,
    __PLUS_expr,
    __NOT_expr,
    __AT_expr,
    __STAR_expr,
    __ROUND_expr,
    __FLOATCAST_expr,
    __expr_HAT_expr,
    __expr_LSQUARE_expr_RSQUARE,
    __PLUSPLUS_expr,
    __expr_PLUSPLUS,
    __MINUSMINUS_expr,
    __expr_MINUSMINUS,
    __IDENTIFIER_LPAREN_exprseq_RPAREN,

    // exprseq -->
    // could be empty or...
    __expr,
    __exprseq_COMMA_expr,
	
    // funcdefn -->
    __FUNCTION_IDENTIFIER_LPAREN_parmseq_RPAREN_body,
    __FUNCTION_IDENTIFIER_LPAREN_parmseq_RPAREN_RETURNS_typeexpr_body,

    // parmdefn -->
    __typeexpr_IDENTIFIER,
    __REF_typeexpr_IDENTIFIER,
	
    // parmseq -->
    // could be empty or...
    __parmdefn,
    __parmseq_COMMA_parmdefn,

    // stmt -->
    __SEMICOLON,
    __READ_expr_SEMICOLON,
    __WRITE_expr_SEMICOLON,
    __expr_EQ_expr_SEMICOLON,
    __IF_expr_THEN_stmt_FI,
    __IF_expr_THEN_stmt_ELSE_stmt_FI,
    __WHILE_expr_DO_stmt_OD,
    __DO_stmt_UNTIL_expr_OD,
    __FOR_EACH_IDENTIFIER_IN_expr_DO_stmt_OD,
    __LCURLY_stmtlist_RCURLY,
    __RETURN_SEMICOLON,
    __RETURN_expr_SEMICOLON,
    __FREE_expr_SEMICOLON,
    __IDENTIFIER_LPAREN_exprseq_RPAREN_SEMICOLON,

    // stmtlist -->
    // could be empty or...
    __stmtlist_stmt,

    // typeexpr -->
    __TYPENAME,
    __ARRAY_OF_typeexpr,
    __POINTER_TO_typeexpr,

    // vardefn -->
    __typeexpr_IDENTIFIER_SEMICOLON,
    __typeexpr_IDENTIFIER_IS_INITIALLY_expr_SEMICOLON
};
//-------------------------------------------------------------------------
#endif

