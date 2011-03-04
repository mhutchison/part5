//----------------------------------------------------------------------------
// File: cu.traverser.cxx
// Written by: Michael Main
// Version: Jan 31, 2011
// This file provides the int traverse( ) function that traverses the
// parse tree provided by the lexer/parser for the cs 3155 language.
// The traversal attaches the attributes that are listed in the project

// Adding a comment to test out github

// Comment #2

// specification.
#include <cassert>            // Provides assert macro
#include <iostream>           // Provides cerr
#include <string>             // Provides the string class
#include "tree.h"             // Provides the tree class
#include "symtab.h"           // Provides the symbol_table class
#include "cu.tab.h"           // Provides the token numbers
#include "cu.enum.h"          // Provides lhs, rhs, and some type functions
using namespace colorado;     // For the tree and symbol_table
using namespace std;
extern tree* parse_tree_root_ptr; // From the parser
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// Prototypes for the functions defined in this file.  Documentation for
// each function is with the function's definition.
void traverse( );
void traverse_subtree(tree* p);
void catch_forbidden_trees(tree* p);
void decorate_identifier(tree* p);
void decorate_nonterminal(tree* p);
int format_of_tt(const tree* tt);
void insert(string name, int kind, tree* value);
bool is_compat(const tree* var_type, const tree* expr_type, bool allow_coercion=true);
void set_Addressable(tree* p);
void set_Bytes(tree* p);
void set_Errors(tree* p);
void set_Type(tree* p);
const tree* tt_constant(const tree* tt);
const tree* tt_minus(const tree* tt, int qualifier);
const tree* tt_plus(const tree* tt, int qualifier);
const tree* tt_primitive(string primitive);
void validate(tree* p);
void validate_call(tree* p);
void validate_expr(tree* p);
void validate_stmt(tree* p);
void validate_vardefn(tree* p);
void write_error(const string& message, tree* p = NULL);
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// A macro that can be called to check whether a condition is valid.
#define check(p,b,fn)							\
    if (!(b))								\
    {write_error(string("Internal compiler error in ") + (fn), (p)); return; }                       
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// Eight constant pointers to constant type trees.  The first four are the
// trees for the four basic types.  The second four are the same, but with
// a constant qualifier added at the front.
const tree* const BOOL_TYPE = tt_primitive("|bool|");
const tree* const FLOAT_TYPE = tt_primitive("|float|");
const tree* const INTEGER_TYPE = tt_primitive("|int|");
const tree* const STRING_TYPE = tt_primitive("|string|");
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// Global variable for the symbol table:
symbol_table<const tree*> st;
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// void traverse( )
// Precondition: parse_tree_root_ptr is pointing to the root of the
// parse tree created by the parser.
// Postcondition: The entire tree has been decorated.  The value of
// parse_tree_root_ptr->attribute<int>("Errors") tells how many total errors
// were found.
void traverse( )
{
    check(
	parse_tree_root_ptr,
	parse_tree_root_ptr->is_attribute<lhs>("LHS") &&
	parse_tree_root_ptr->attribute<lhs>("LHS") == program__,
	"traverse"
	);

    // Initialize symbol table and register the different kinds of symbols:
    st.clear( );
    st.register_kind(funcdefn__, 0, 1, false);
    st.register_kind(vardefn__, -4, -4, true);
    st.register_kind(parmdefn__, 16, 4, true);

    // Open up the global scope, decorate the tree, and exit
    st.enter_scope( );
    traverse_subtree(parse_tree_root_ptr);
    st.exit_scope( );
}
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// void traverse_subtree(tree* p)
// Sets attributes for the the root node of p and all of its subtrees.
void traverse_subtree(tree* p)
{

    lhs what;  // The nonterminal at the root of this tree
    rhs right; // The rhs at the root of this tree
    size_t i;  // Index for one of the subtrees
    size_t many = p->many_children( ); // How many subtrees

    if (p->is_attribute<lhs>("LHS"))
    {   // Decorate the subtrees of p before decorating the root.
	what = p->attribute<lhs>("LHS");

	// Function definitions add the function name to the symbol
	// table and then open a new scope.
	switch (what)
	{
	case funcdefn__:
	    // A funcdefn is inserted here, so that it's available
	    // when we process the function's body.
	    insert(p->child(1)->label( ), what, p);
	    st.enter_scope( );
	    break;
	default:
	    break;
	}

	// Decorate all the subtrees:
	for (i = 0; i < many; ++i)
	    traverse_subtree(p->child(i));
	    
	// Function definitions close the scope. Variable or type
	// definitions add the newly defined name to the symbol table.
	switch(what)
	{
	case funcdefn__:
	    st.exit_scope( );
	    break;
	case vardefn__:
	case parmdefn__:
	    // A variable definition is not inserted until after we have
	    // processed the initialization part of the definition.
	    right = p->attribute<rhs>("RHS");
	    i = (right == __REF_typeexpr_IDENTIFIER) ? 2 : 1;
	    insert(p->child(i)->label( ), what, p);
	    decorate_identifier(p->child(i)); // Redecorate now it's inserted
	    break;
	default:
	    break;
	}
	
        // Set all other required attributes:
	decorate_nonterminal(p);
    }
    else if ((p->attribute<int>("Token")) == IDENTIFIER)
	decorate_identifier(p);
}
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// void catch_forbidden_trees(tree* p)
// Checks for three forbidden parse trees.  If one of them is found, then an
// error message is printed and p->attribute<int>("Errors") is incremented.
// The errors are:
// 1. A comma at the start of an exprseq or parmseq.
// 2. A typename that is not one of the four primitive names (|int|,
//    |float|, |bool|, or |string|.
// 3. for each statements are not yet allowed.
void catch_forbidden_trees(tree* p)
{
    string name;
    
    switch(p->attribute<rhs>("RHS"))
    {
    case __exprseq_COMMA_expr:
    case __parmseq_COMMA_parmdefn:
	// Forbid a comma at the front of a sequence.
	if (p->child(0)->attribute<rhs>("RHS") == __EMPTY)
	    write_error("A comma is forbidden at the start of a sequence", p);
	break;
    case __TYPENAME:
	name = p->child(0)->label( );
	if (name == "|int|") break;
	else if (name == "|float|") break;
	else if (name == "|string|") break;
	else if (name == "|bool|") break;
	write_error("Unknown typename: " + name, p);
	break;
    case __FOR_EACH_IDENTIFIER_IN_expr_DO_stmt_OD:
	write_error("The for-each statement is not yet supported.", p);
	break;
    case __NULLPTR:
	write_error("The NULLPTR value is not yet supported.", p);
	break;
    default:
	break;
    }
}
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// void decorate_identifier(tree* p)
// Preconditiion: p is a terminal node with the IDENTIFIER token.
// Postcondition: The Definition, Depth, Kind and Offset attributes of
// p have been set.
void decorate_identifier(tree* p)
{
    check(p, p->attribute<int>("Token") == IDENTIFIER, "decorate_identifier");
    string& name = p->label( );
    const tree* defn;
    
    if (st.seek(name))
    {
	defn = st.value( );
	p->set_attribute<const tree*>("Definition", defn);
	p->set_attribute<int>("Depth", st.depth( ));
	p->set_attribute<lhs>("Kind", lhs(st.kind( )));
	p->set_attribute<int>("Offset", st.offset( ));
	p->set_attribute<bool>(
	    "Reference",
	    defn->attribute<rhs>("RHS") == __REF_typeexpr_IDENTIFIER
	    );
    }
    else if (
	p->parent( )->attribute<lhs>("LHS") == expr__
        ||
	p->parent( )->attribute<lhs>("LHS") == stmt__
	)
    {
	p->set_attribute<const tree*>("Definition", NULL);
	p->set_attribute<lhs>("Kind", ILLEGAL_NONTERMINAL);
	write_error("Unknown identifier: " + name, p);
    }
}
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// void decorate_nonterminal(tree* p)
// Precondition: p is a nonterminal node.
// Postcondition: All required attributes for p have been set.  The exact
// kinds of attributes depend on the LHS of p as defined in Guide 3 for
// the CSCI 3155 Language.
void decorate_nonterminal(tree* p)
{
    check(p, p->is_attribute<lhs>("LHS"), "decorate_nonterminal");

    set_Errors(p);             // Adds sum of child errors to Error attribute
    catch_forbidden_trees(p);  // Catches forbidden parse trees
    validate(p);               // Checks that types of children are okay
    switch (p->attribute<lhs>("LHS"))
    {
    case expr__:
	set_Addressable(p);    // Sets the Addressable attribute of an expr
	set_Type(p);           // Sets the Type attribute of an expr
	break;
    case defnlist__:
    case parmseq__:
    case program__:
	set_Bytes(p);          // Sets the Bytes attribute of a definition
	break;
    default:
	break;
    }
}
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// int format_of_tt(const tree* tt)
// Precondition: tt must be a type tree (i.e., a parse tree with lhs of
// typeexpr__).
// Return value: The token number for the child 0.
// This could be TYPENAME, ARRAY, POINTER.
// If tt is the null pointer, then the return value is the NULLPTR token.
// In effect, format_of_tt tells you what format the type is at the top level.
int format_of_tt(const tree* tt)
{
    if (tt == NULL) return NULLPTR;
    return tt->child(0)->attribute<int>("Token");
}
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// void insert(string name, int kind, tree* value)
// This function tries to insert the specified name into the symbol table
// with the given kind and value parameters.  If the symbol table insert
// function indicates an error, then an error message is printed and the
// error count of the type tree is incremented.
void insert(string name, int kind, tree* value)
{
    if (!st.insert(name, kind, value))
	write_error("Duplicate identifier declared", value);
}
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// bool is_compat
// (const tree* var_type, const tree* expr_type, bool allow_coercion)
// Precondition: Each parameter must be a portion of a parse
// tree with typeexpr at the top.
// Return value: The return value is true if an expression of type expr_type
// can be used to initialize a variable of type var_type. If allow_coercion
// is true, then a coercion from int to float is permitted.
bool is_compat(const tree* var_type, const tree* expr_type, bool allow_coercion)
{
    int var_format, expr_format; // Token of first child
    string var_primitive, expr_primitive; // Lexeme for type name

    // The type of nullptr is NULL, and it can work for any ptr variable:
    if (expr_type == NULL)
	return (format_of_tt(var_type) == POINTER);

    // Check that their top levels are the same:
    var_format = format_of_tt(var_type);
    expr_format = format_of_tt(expr_type);
    if (var_format != expr_format)
	return false;
    
    switch (var_format)
    {
    case TYPENAME:
	var_primitive = var_type->child(0)->label( );
	expr_primitive = expr_type->child(0)->label( );
	if (
	    allow_coercion
	    &&
	    var_primitive == "|float|"
	    &&
	    expr_primitive == "|int|"
	    )
	    return true;
	return var_primitive == expr_primitive;

    case ARRAY:
	expr_type = tt_minus(expr_type, ARRAY);
	var_type = tt_minus(var_type, ARRAY);
	return is_compat(var_type, expr_type, allow_coercion);

    case POINTER:
	expr_type = tt_minus(expr_type, POINTER);
	var_type = tt_minus(var_type, POINTER);
	return is_compat(var_type, expr_type, false);
	
    default:
	write_error("Internal compiler error in is_init_compat");
	return false;
    }
}
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// void set_Addressable(tree* p)
// Precondition: expr_tree is a portion of the parse tree with expr at the top.
// All of its children have already been decorated.
// Postcondition: The Addressable attribute of the expr has been set.
void set_Addressable(tree* p)
{   
    check(p, p->is_attribute<lhs>("LHS"), "set_Addressable");
    check(p, p->attribute<lhs>("LHS") == expr__, "set_Addressable");

    bool answer;

    switch(p->attribute<rhs>("RHS"))
    {
    case __IDENTIFIER:
    case __STAR_expr:
    case __PLUSPLUS_expr:
    case __MINUSMINUS_expr:
	answer = true;
	break;
    case __LPAREN_expr_RPAREN:
	answer = p->child(1)->attribute<bool>("Addressable");
	break;
    case __expr_LSQUARE_expr_RSQUARE:
        // In order to guarantee that this element has an address, we
	// must have access to the address of the array.
	answer = p->child(0)->attribute<bool>("Addressable");
	break;
    default:
	answer = false;
	break;
    }
    
    p->set_attribute<bool>("Addressable", answer);
    
}
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// void set_Bytes(tree* p)
// Precondition: p is a portion of the parse tree with defnlist, parmseq or
// program at the top.
// Postcondition: The Bytes attribute of p has been set.
void set_Bytes(tree* p)
{
    check(p, p->is_attribute<lhs>("LHS"), "set_Bytes");
    
    switch (p->attribute<lhs>("LHS"))
    {
    case defnlist__:
	p->set_attribute<int>("Bytes", st.many_bytes(vardefn__));
	break;
    case parmseq__:
	// Set attributes: Bytes
	p->set_attribute<int>("Bytes", st.many_bytes(parmdefn__));
	break;
    case program__:
        // Set attributes: Bytes
	p->set_attribute<int>
        ("Bytes", p->child(0)->attribute<int>("Bytes"));
	break;
    default:
	write_error("Internal compiler error in set_Bytes", p);
    }
}
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// void set_Errors(tree* p)
// Postcondition: The Errors attribute of p has been incremented by the sum
// of the Error attributes of its children.
void set_Errors(tree* p)
{
    int& errors = p->attribute<int>("Errors");
    int i;
    int many = p->many_children( );

    for (i = 0; i < many; ++i)
        errors += p->child(i)->attribute<int>("Errors");
}
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// void set_Type(tree* p)
// Precondition: expr_tree is a portion of the parse tree with expr at the top.
// All of its children have already been decorated.
// Postcondition: The Type attribute of the expr has been set.
void set_Type(tree* p)
{
    check(p, p->is_attribute<lhs>("LHS"), "set_Type");
    check(p, p->attribute<lhs>("LHS") == expr__, "set_Type"); 

    const tree* answer = NULL; // The type of this expression
    const tree* t1;            // The type of a subexpression
    const tree* t2;            // The type of another subexpression

    if (p->attribute<int>("Errors") == 0) switch(p->attribute<rhs>("RHS"))
	{
	case __INTEGERVALUE:
	case __expr_PERCENT_expr:
	case __ROUND_expr:
	    answer = INTEGER_TYPE;
	    break;
	case __FLOATVALUE:
	case __FLOATCAST_expr:
	    answer = FLOAT_TYPE;
	    break;
	case __STRINGVALUE:
	    answer = STRING_TYPE;
	    break;
	case __NULLPTR:
	    answer = NULL;
	    break;
	case __TRUE:
	case __FALSE:
	case __expr_OR_expr:
	case __expr_AND_expr:
	case __expr_EQEQ_expr:
	case __expr_NE_expr:
	case __expr_LT_expr:
	case __expr_GT_expr:
	case __expr_LE_expr:
	case __expr_GE_expr:
	case __NOT_expr:
	    answer = BOOL_TYPE;
	    break;
	case __IDENTIFIER:
	    t1 = p->child(0)->attribute<const tree*>("Definition");
	    if (t1->attribute<rhs>("RHS") == __REF_typeexpr_IDENTIFIER)
		answer = t1->child(1); // for a reference parameter
	    else
		answer = t1->child(0); // for any other definition
	    break;
	case __LPAREN_ARRAY_OF_typeexpr_IS_exprseq_RPAREN:
	    // This is a constant array.  The component type is given in
	    // the typeexpr.
	    answer = tt_plus(p->child(3), ARRAY);
	    break;	
	case __LPAREN_NEW_typeexpr_IS_expr_RPAREN:
	    // Set answer to the type of the newly allocated var plus pointerto:
	    answer = tt_plus(p->child(2), POINTER);
	    break;
	case __LPAREN_expr_RPAREN:
	    answer = p->child(1)->attribute<const tree*>("Type");
	    break;
	case __expr_PLUS_expr:
	case __expr_MINUS_expr:
	case __expr_STAR_expr:
	case __expr_SLASH_expr:
	case __expr_HAT_expr:
	    // If both children are |int|, then so is answer:
	    t1 = p->child(0)->attribute<const tree*>("Type");
	    t2 = p->child(2)->attribute<const tree*>("Type");
	    if (is_compat(INTEGER_TYPE, t1) && is_compat(INTEGER_TYPE, t2))
		answer = INTEGER_TYPE;
	    else
		answer = FLOAT_TYPE;
	    break;
	case __MINUS_expr:
	case __PLUS_expr:
	    // If the child is |int|, then so is answer:
	    t1 = p->child(1)->attribute<const tree*>("Type");
	    if (is_compat(INTEGER_TYPE, t1))
		answer = INTEGER_TYPE;
	    else
		answer = FLOAT_TYPE;
	    break;
	case __AT_expr:
	    answer = tt_plus(p->child(1)->attribute<const tree*>("Type"), POINTER);
	    break;
	case __STAR_expr:
	    // Set answer to the expr's type minus pointer to
	    t1 = p->child(1)->attribute<const tree*>("Type");
	    answer = tt_minus(t1, POINTER);
	    break;
	case __expr_LSQUARE_expr_RSQUARE:
	    // Set answer to the array's type minus array of:
	    t1 = p->child(0)->attribute<const tree*>("Type");
	    answer = tt_minus(t1, ARRAY);
	    break;
	case __PLUSPLUS_expr:
	case __MINUSMINUS_expr:
	    answer = INTEGER_TYPE;
	    break;
	case __expr_PLUSPLUS:
	case __expr_MINUSMINUS:
	    answer = INTEGER_TYPE;
	    break;
	case __IDENTIFIER_LPAREN_exprseq_RPAREN:
	    // Set answer to the return type of the function
	    t1 = p->child(0)->attribute<const tree*>("Definition");
	    t2 = t1->child(6);
	    answer = t2;
	    break;
	default:
	    write_error("Internal compiler error in set_Type", p);
	}
	    
    p->set_attribute<const tree*>("Type", answer);
}
//-------------------------------------------------------------------------


//----------------------------------------------------------------------------
// const tree* tt_minus(const tree* tt, int qualifier)
// Precondition: tt must be a type tree (i.e., a parse tree with lhs of
// typeexpr__).
// Return value: a pointer to a subtree of tt that no longer has tt's
// top-level qualifier (determined by the qualifier parameter, which may be
// ARRAY or POINTER). If tt does not have that qualifier parameter at its top,
// then the return value is NULL.
const tree* tt_minus(const tree* tt, int qualifier)
{
    const tree* answer = tt;

    if (answer->child(0)->attribute<int>("Token") == qualifier)
	return answer->child(2);
    else
	return NULL;
}
//----------------------------------------------------------------------------


//-------------------------------------------------------------------------
// const tree* tt_plus(const tree* tt, int qualifier)
// Precondition: tt must be a type tree (i.e., a parse tree with lhs of
// typeexpr__); qualifier must be ARRAY or POINTER>.
// Return value: a pointer to a const tree that is a copy of tt
// with these additions:
// 1. The qualifier has been added to the front of the type (the
//    top of the type tree) along with a second OF or TO node.
// 2. If constant is true, then a constant qualifier is added to
//    the front of the type.
const tree* tt_plus(const tree* tt, int qualifier)
{
    tree* tt_copy = new tree(*tt);
    tree* answer;
    tree* child;
    tree* child2;

    child = new tree( );
    child->set_attribute<int>("Token", qualifier);
    child->set_attribute<int>("Line", 0);
    child->set_attribute<int>("Errors", 0);
    
    child2 = new tree( );
    child2->set_attribute<int>("Line", 0);
    child2->set_attribute<int>("Errors", 0);
    
    answer = new tree("<typeexpr>", 3, child, child2, tt_copy);
    answer->set_attribute<lhs>("LHS", typeexpr__);
    answer->set_attribute<int>("Line", 0);
    answer->set_attribute("Errors", 0);
    
    switch (qualifier)
    {
    case ARRAY:
	child->set_label("array");
	child2->set_label("of");
	child2->set_attribute<int>("Token", OF);
	answer->set_attribute<rhs>("RHS", __ARRAY_OF_typeexpr);
	break;
    case POINTER:
	child->set_label("pointer");
	child2->set_label("to");
	child2->set_attribute<int>("Token", TO);
	answer->set_attribute<rhs>("RHS", __POINTER_TO_typeexpr);
	break;
    default:
	break;
    }

    return answer;
}
//-------------------------------------------------------------------------


//-------------------------------------------------------------------------
// const tree* tt_primitive(string primitive)
// The return value is a type tree for one of the basic types
// (|int|, |float|, |bool|, |string|).
const tree* tt_primitive(string primitive)
{
    tree *leaf;
    tree *root;

    leaf = new tree(primitive);
    leaf->set_attribute<int>("Token", TYPENAME);
    leaf->set_attribute<tree*>("Definition", NULL);
    root = new tree("<typeexpr>", 1, leaf);
    root->set_attribute<lhs>("LHS", typeexpr__);
    root->set_attribute<rhs>("RHS", __TYPENAME);

    return root;
}
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// void validate(tree* p)
// Postcondition: The types of each of p's children have been type
// checked.  If any errors were found, then an error message is printed and
// the Error attribute of p has been incremented.
void validate(tree* p)
{
    check(p, p->is_attribute<lhs>("LHS"), "validate");

    if (p->attribute<int>("Errors") > 0) return;
    
    switch(p->attribute<lhs>("LHS"))
    {
    case expr__:
	validate_expr(p);
	break;
    case stmt__:
	validate_stmt(p);
	break;
    case vardefn__:
	validate_vardefn(p);
	break;
    default:
	break;
    }

}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// void validate_arguments(tree* p, const tree* ppl, const tree* pal)
// Precondition: p is a parse tree for a function call in which the number
// of arguments (from the exprseq in pal) equals the number of parameters
// (from the parmseq in ppl).
// Postcondition: The types of the arguments have been checked with the
// parameters.
void validate_arguments(tree* p, const tree* ppl, const tree* pal)
{
    const tree* pp, *pa;   // Pointers to one parmdefn or expr (argument)
    const tree* ppt, *pat; // Pointers to the types of parameter or argument
    bool is_ref;           // True if a parameter is a reference parameter

    check(p, ppl->attribute<lhs>("LHS") == parmseq__, "validate_arguments");
    check(p, pal->attribute<lhs>("LHS") == exprseq__, "validate_arguments");

    if (ppl->many_children( ) == 3)
    {   // At least two parameters (RHS is __parmseq_COMMA_parmdefn
	validate_arguments(p, ppl->child(0), pal->child(0));
    }

    if (ppl->many_children( ) > 0)
    {   // Check the final parameter against the final argument.
	pp = ppl->child(ppl->many_children()-1); // pp points to a parmdefn
	pa = pal->child(pal->many_children()-1); // pa points to an expr
	is_ref = pp->attribute<rhs>("RHS") == __REF_typeexpr_IDENTIFIER;
	ppt = pp->child(is_ref ? 1 : 0);         // pointer to a typeexpr for parameter
	pat = pa->attribute<const tree*>("Type");// pointer to a typeexpr for argument
	
	if (is_ref)
	{   // Reference parameter
	    if (!pa->attribute<bool>("Addressable"))
		write_error("L-value expected", p);
	    if (!is_compat(ppt, pat, false))
		write_error("Invalid argument type", p);
	}
	else
	{   // Value parameter
	    if (!is_compat(ppt, pat, true))
		write_error("Invalid argument type", p);
	}
    }
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// int seq_length(const tree* p)
// Precondition: p is a pointer to an exprseq or parmseq parse tree.
// Postcondition: The return value is the number of expr or parm in the tree.
int seq_length(const tree* p)
{
    switch (p->attribute<rhs>("RHS"))
    {
    case __EMPTY:
	return 0;
    case __expr:
    case __parmdefn:
	return 1;
    case __exprseq_COMMA_expr:
    case __parmseq_COMMA_parmdefn:
	return seq_length(p->child(0)) + 1;
    default:
	return 0;
    }
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// void validate_call(tree* p)
// Postcondition: The types of each of p's children have been type
// checked.  If any errors were found, then an error message is printed and
// the Error attribute of p has been incremented.
void validate_call(tree* p)
{
    check(p, p->is_attribute<rhs>("RHS"), "validate_call");
    rhs what = p->attribute<rhs>("RHS");
    check(p,
	  what == __IDENTIFIER_LPAREN_exprseq_RPAREN
	  ||
	  what == __IDENTIFIER_LPAREN_exprseq_RPAREN_SEMICOLON,
	  "validate_call"
	);

    const tree* pfd = p->child(0)->attribute<const tree*>("Definition");
    const tree* pal = p->child(2);    // Pointer to exprseq
    const tree* ppl = pfd->child(3);  // Pointer to parmseq

    if (seq_length(pal) != seq_length(ppl))
    {
	write_error("Wrong number of arguments in function call", p);
	return;
    }

    validate_arguments(p, ppl, pal);
}
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// void validate_expr(tree* p)
// Precondition: p is a pointer to an expr.
// Postcondition: The types of each of p's children have been type
// checked.  If any errors were found, then an error message is printed and
// the Error attribute of p has been incremented.
void validate_expr(tree* p)
{
    const tree* defn;
    const tree* subtree;
    const tree* t1;
    const tree* t2;
    
    check(p, p->is_attribute<lhs>("LHS"), "validate_expr");
    check(p, p->attribute<lhs>("LHS") == expr__, "validate_expr");

    switch(p->attribute<rhs>("RHS"))
    {
    case __IDENTIFIER:
	// The IDENTIFIER's defn must not be a funcdefn
	defn = p->child(0)->attribute<const tree*>("Definition");
	if (defn->attribute<lhs>("LHS") == funcdefn__)
	    write_error("Function name used illegally", p);
	break;
    case __LPAREN_ARRAY_OF_typeexpr_IS_exprseq_RPAREN:
	// The type of every expression in the exprseq must be
	// compatible with the type of the array's elements.
	t1 = p->child(3);
	subtree = p->child(5);
	while (subtree->attribute<lhs>("LHS") == exprseq__ && subtree->attribute<rhs>("RHS") != __EMPTY)
	{
	    t2 = subtree->child(subtree->many_children( )-1)->attribute<const tree*>("Type");
	    if (!is_compat(t1, t2))
		write_error("Type error in array initialization", p);
	    subtree = subtree->child(0);
	}
	break;
    case __LPAREN_NEW_typeexpr_IS_expr_RPAREN:
	// The type of the initialization expression must be
	// compatible with the type of the new heap-dynamic variable.
	t1 = p->child(2);
	t2 = p->child(4)->attribute<const tree*>("Type");
	if (!is_compat(t1, t2))
	    write_error("Type error in heap-dynamic variable initialization", p);
	break;
    case __expr_OR_expr:
    case __expr_AND_expr:
	// The type of both exprs must be compatible with bool.
	t1 = p->child(0)->attribute<const tree*>("Type");
	t2 = p->child(2)->attribute<const tree*>("Type");
	if (is_compat(BOOL_TYPE, t1) && is_compat(BOOL_TYPE, t2))
	    break;
	write_error("Type error in expression", p);
	break;
    case __expr_EQEQ_expr:
    case __expr_NE_expr:
    case __expr_LT_expr:
    case __expr_GT_expr:
    case __expr_LE_expr:
    case __expr_GE_expr:
	// The type of both exprs must be compatible with the same basic type.
	t1 = p->child(0)->attribute<const tree*>("Type");
	t2 = p->child(2)->attribute<const tree*>("Type");
	if (is_compat(BOOL_TYPE, t1) && is_compat(BOOL_TYPE, t2))
	    break;
	if (is_compat(STRING_TYPE, t1) && is_compat(STRING_TYPE, t2))
	    break;
	if (is_compat(INTEGER_TYPE, t1) && is_compat(INTEGER_TYPE, t2))
	    break;
	if (is_compat(FLOAT_TYPE, t1) && is_compat(FLOAT_TYPE, t2))
	    break;
	write_error("Type error in expression", p);
	break;
    case __expr_PLUS_expr:
    case __expr_MINUS_expr:
    case __expr_STAR_expr:
    case __expr_SLASH_expr:
    case __expr_HAT_expr:
	// The type of both exprs must be compatible with float (which could be int)
	t1 = p->child(0)->attribute<const tree*>("Type");
	t2 = p->child(2)->attribute<const tree*>("Type");
	if (is_compat(FLOAT_TYPE, t1) && is_compat(FLOAT_TYPE, t2))
	    break;
	write_error("Type error in expression", p);
	break;
    case __expr_PERCENT_expr:
	// The type of both exprs must be compatible with integer.
	t1 = p->child(0)->attribute<const tree*>("Type");
	t2 = p->child(2)->attribute<const tree*>("Type");
	if (is_compat(INTEGER_TYPE, t1) && is_compat(INTEGER_TYPE, t2))
	    break;
	write_error("Type error in expression", p);
	break;
    case __MINUS_expr:
    case __PLUS_expr:
    case __FLOATCAST_expr:
    case __ROUND_expr:
	// The type of the expression must be compatible with float (which could be int)
	t1 = p->child(1)->attribute<const tree*>("Type");
	if (is_compat(FLOAT_TYPE, t1))
	    break;
	write_error("Type error in expression", p);
	break;
    case __NOT_expr:
	// The type of the expr must be compatible with bool.
	t1 = p->child(1)->attribute<const tree*>("Type");
	if (is_compat(BOOL_TYPE, t1))
	    break;
	write_error("Type error in expression", p);
	break;
    case __STAR_expr:
	// The type of the expr must be a pointer to something.
	t1 = p->child(1)->attribute<const tree*>("Type");
	if (format_of_tt(t1) != POINTER)
	    write_error("Pointer expected for * operator", p);
	break;
    case __AT_expr:
	// The type of the expr must be addressable.
	if (!p->child(1)->attribute<bool>("Addressable"))
	    write_error("Attempted to create pointer to non-addressable value", p);
	break;
    case __expr_LSQUARE_expr_RSQUARE:
	// The type of the first expr must be an arrayof something.
	// The type of the second expr must be compatible with integer.
	t1 = p->child(0)->attribute<const tree*>("Type");
	t2 = p->child(2)->attribute<const tree*>("Type");
	if (format_of_tt(t1) != ARRAY)
	    write_error("Attempted use of non-array expression as an array", p);
	if (!is_compat(INTEGER_TYPE, t2))
	    write_error("Array index must be an integer", p);
	break;
    case __PLUSPLUS_expr:
    case __MINUSMINUS_expr:
	// The type of the expression must be addressable and compatible with int.
	if (!p->child(1)->attribute<bool>("Addressable"))
	    write_error("Increment or decrement of non-addressable value", p);
	t1 = p->child(1)->attribute<const tree*>("Type");
	if (is_compat(INTEGER_TYPE, t1))
	    break;
	write_error("Type error in expression", p);
	break;
    case __expr_PLUSPLUS:
    case __expr_MINUSMINUS:
	// The type of the expression must be addressable and compatible with int.
	if (!p->child(0)->attribute<bool>("Addressable"))
	    write_error("Increment or decrement of non-addressable value", p);
	t1 = p->child(0)->attribute<const tree*>("Type");
	if (is_compat(INTEGER_TYPE, t1))
	    break;
	write_error("Type error in expression", p);
	break;
    case __IDENTIFIER_LPAREN_exprseq_RPAREN:
	// The function definition must have a return value, and the arguments must
	// be compatible with the function's parameters.
	defn = p->child(0)->attribute<const tree*>("Definition");
	if (defn->attribute<lhs>("LHS") != funcdefn__)
	    write_error("Function name expected", p);
	else if (defn->many_children( ) != 8)
	{
	    write_error("Function call used in expression must return a value", p);
	}
	else
	    validate_call(p);
	break;
    default:
	break;
    }
}
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// void validate_stmt(tree* p)
// Postcondition: The types of each of p's children have been type
// checked.  If any errors were found, then an error message is printed and
// the Error attribute of p has been incremented.
void validate_stmt(tree* p)
{
    const tree* t1;
    const tree* t2;
    const tree* defn;
    
    check(p, p->is_attribute<lhs>("LHS"), "validate_stmt");
    check(p, p->attribute<lhs>("LHS") == stmt__, "validate_stmt");

    switch(p->attribute<rhs>("RHS"))
    {
    case __READ_expr_SEMICOLON:
	t1 = p->child(1)->attribute<const tree*>("Type");
	if (!p->child(1)->attribute<bool>("Addressable"))
	    write_error("Operand for \"read\" must be addressable", p);
	if (format_of_tt(t1) != TYPENAME)
	    write_error("Operand for \"read\" must be a primitive type", p);
	break;
    case __WRITE_expr_SEMICOLON:
	t1 = p->child(1)->attribute<const tree*>("Type");
	if (format_of_tt(t1) != TYPENAME)
	    write_error("Operand for \"write\" must be a primitive type", p);
	break;
    case __expr_EQ_expr_SEMICOLON:
	t1 = p->child(0)->attribute<const tree*>("Type");
	t2 = p->child(2)->attribute<const tree*>("Type");
	if (!p->child(0)->attribute<bool>("Addressable"))
	    write_error("Error: Left-side of assignment must be addressable", p);
	if (!is_compat(t1, t2))
	    write_error("Type error in right-side of assignment operator", p);
	break;
    case __IF_expr_THEN_stmt_FI: 
    case __IF_expr_THEN_stmt_ELSE_stmt_FI:
	t1 = p->child(1)->attribute<const tree*>("Type");
	if (!is_compat(BOOL_TYPE, t1))
	    write_error("Bool expression expected if if-statement", p);
	break;
    case __WHILE_expr_DO_stmt_OD:
	t1 = p->child(1)->attribute<const tree*>("Type");
	if (!is_compat(BOOL_TYPE, t1))
	    write_error("Bool expression expected in while-statement", p);
	break;
    case __DO_stmt_UNTIL_expr_OD:
	t1 = p->child(3)->attribute<const tree*>("Type");
	if (!is_compat(BOOL_TYPE, t1))
	    write_error("Bool expression expected in do-statement", p);
	break;
    case __RETURN_SEMICOLON:
	st.seek(st.current_depth( )-1);
	if (st.value( )->many_children( ) == 8)
	    write_error("Return statement in function requires a value", p);
	break;
    case __RETURN_expr_SEMICOLON:
	st.seek(st.current_depth( )-1);
	if (st.value( )->many_children( ) == 6)
	{
	    write_error("Return statement in procedure forbids a value", p);
	    break;
	}
	t1 = st.value( )->child(6); // Function's return type
	t2 = p->child(1)->attribute<const tree*>("Type");
	if (!is_compat(t1, t2))
	    write_error("Wrong data type for return statement", p);
	break;
    case __FREE_expr_SEMICOLON:
	t1 = p->child(1)->attribute<const tree*>("Type");
	if (format_of_tt(t1) != POINTER)
	    write_error("Pointer expected for release operator", p);
	break;
    case __IDENTIFIER_LPAREN_exprseq_RPAREN_SEMICOLON:
	defn = p->child(0)->attribute<const tree*>("Definition");
	if (defn->attribute<lhs>("LHS") != funcdefn__)
	    write_error("Function name expected", p);
	else
	    validate_call(p);
	break;
    default:
	break;
    }
}
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// void validate_vardefn(tree* p)
// Precondition: p is a pointer to a vardefn.
// Postcondition: The types of each of p's children have been type
// checked.  If any errors were found, then an error message is printed and
// the Error attribute of p has been incremented.
void validate_vardefn(tree* p)
{
    check(p, p->is_attribute<lhs>("LHS"), "validate_vardefn");
    check(p, p->attribute<lhs>("LHS") == vardefn__, "validate_defn");

    if (p->many_children( ) > 3)
	if (!is_compat(p->child(0), p->child(4)->attribute<const tree*>("Type")))
	    write_error("Type error in variable initialization", p);
}
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// void write_error(const string& message, tree* p)
// The message has been printed to cerr as an ERROR message.  If p is not
// NULL, then it's Error count has been incremented.
void write_error(const string& message, tree* p)
{
    cerr << "ERROR";
    if (p != NULL)
	cerr << " on line " << p->attribute<int>("Line");
    cerr << " -- " << message << endl;
    if (p != NULL)
	++(p->attribute<int>("Errors"));
}
//----------------------------------------------------------------------------

