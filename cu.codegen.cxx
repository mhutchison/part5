//-----------------------------------------------------------------------------
// File: cu.codegen.cxx
// Written by Michael Main (Version: Feb 26, 2011)
// Modified by: Your Name(s)
// See http://www.portmain.com/pl/guides/ for
// additional discussion and documentation.
//
// This file provides the codegen( ) function that traverses the parse
// tree (*parse_tree_root_ptr) produced by the lexer/parser/decorator for the
// CU language. The traversal outputs assembly code for the input CU
// program. The input comes from standard input (cin).  The assembly output
// goes to standard output (cout).  Error output goes to error output (cerr).

// Windows or Linix directions:
// To make the entire compiler with debugging checks: make hw7
// After compilation, a CU program xxx.cu can be compiled to create
// xxx.exe (in Windows) or an executable xxx file (in Linux) with:
//   cu <xxx.cu >xxx.s
//   g++ -m32 xxx.s -o xxx
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Includes and directives
#include <cassert>        // Provides assert macro
#include <cstdio>         // Provides sprintf
#include <fstream>        // Provides ifstream
#include <iomanip>        // Provides setw
#include <iostream>       // Provides cerr and cout
#include <queue>          // Provides queue
#include <string>         // Provides the string class

// if error "atof not declared":
// include other header files for this function

#include "tree.h"         // Provides the tree class
#include "cu.tab.h"       // Provides the token numbers
#include "cu.enum.h"      // Provides lhs, rhs
using namespace std;
using namespace colorado;

// Some type functions from cu.traverser.cxx
extern int format_of_tt(const tree* tt);
extern bool is_compat(const tree* var_type, const tree* expr_type, bool allow_coercion = true);
extern int seq_length(const tree* p);
extern const tree* const tt_primitive(string primitive); 
extern const tree* tt_minus(const tree* tt, int qualifier);
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Functions and MACROS for assembly instructions that we will use.
// Note that we are using 32-bit machine instructions only (the -m32 option).
#define ASM_COMMAND "\n#  gcc -m32 xxx.s -o xxx\n"
const int MAX_OPERAND = 34;        // Maximum length of an assembly operand
const int TAB = 35;                // Position of tab in assembly commands
#define ADD(op1, op2, comment) print_instruction(comment, "addl", op1, op2)
#define ALLOCATE_STACK(amount, comment) print_instruction(comment, "subl", amount, "%esp")
#define CALL(function, comment) print_instruction(comment, "call", function)
#define CMP(op1, op2, comment) print_instruction(comment, "cmpl", op1, op2)
#define CDQ(comment) print_instruction(comment, "cdq")
#define DEC(op, comment) print_instruction(comment, "decl", op)
#define IDIV(op, comment) print_instruction(comment, "idivl", op)
#define IMUL(op, comment) print_instruction(comment, "imull", op)
#define INC(op, comment) print_instruction(comment, "incl", op)
#define JUMP(jxx, j, comment) print_instruction(comment, jxx, jump_label(j))
#define LABEL(j) cout << "  " << jump_label(j) << ":" << endl
#define LONG(op, comment) cout << "  .long " << setw(TAB-8) << (op) << " # " << (comment) << endl
#define MOV(op1, op2, comment) print_instruction(comment, "movl", op1, op2)
#define NEG_TOP print_instruction("(%esp) = -1*(%esp)", "negl", "(%esp)")
#define NOT_TOP print_instruction("Flips between 0 and 1", "xorl", 1, "(%esp)")
#define POP(op, comment) print_instruction(comment, "popl", op)
#define PUSH(op, comment) print_instruction(comment, "pushl", op)
#define RELEASE_STACK(amount, comment) print_instruction(comment, "addl", amount, "%esp")
#define RET(comment) print_instruction(comment, "ret")
#define SHL(amount, op, comment) print_instruction(comment, "shll", amount, op)
#define SHR(amount, op, comment) print_instruction(comment, "shrl", amount, op)
#define SUB(amount, op, comment) print_instruction(comment, "subl", amount, op)

#define FADD(op, comment) print_instruction(comment, "fadd", op)
#define FCHS(comment) print_instruction(comment, "fchs")
#define FILD(op, comment) print_instruction(comment, "fild", op)
#define FISTPL(op, comment) print_instruction(comment, "fistpl", op)
#define FLD(op, comment) print_instruction(comment, "fld", op)
#define FLDZ(comment) print_instruction(comment, "fldz")
#define FSTP(op, comment) print_instruction(comment, "fstp", op)
#define FSTP8(op, comment) print_instruction(comment, "fstpl", op)
#define FSUB(op, comment) print_instruction(comment, "fsub", op)
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
#define check(b,fn)     \
  if (!(b))             \
    {cerr << "ERROR -- Internal compiler error in " << (fn) << endl; return; }
//----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function prototypes.  Please see each individual function for its
// documentation.  In general, a function name's first few letters tells its
// purpose.
//
//   cg_:
//   generate assembly code for node in the parse tree with a particular lhs.
//   Some of these functions also require a particular rhs, which is indicated
//   by the part of the function name that follows two underscores.
//   For example, cg_stmt__READ_expr_SEMICOLON generates code for an expression
//   that has __READ_expr_SEMICOLON as its rhs.
//
//   cgx_:
//   generate extra assembly code that is useful in a variety of situations.
//
// For debugging purposes, each of the cg functions begins with a check to
// ensure that the parameter is a pointer to the right kind of tree.
void codegen(const tree* p); // The main code generator.

void cd_funcdefn(const tree* p);
void cg_program(const tree* p);
void cg_stmt(const tree* p);
void cg_stmt__IF_expr_THEN_stmt_FI(const tree* p);
void cg_stmt__IF_expr_THEN_stmt_ELSE_stmt_FI(const tree* p);
void cg_stmt__WHILE_expr_DO_stmt_OD(const tree* p);
void cg_stmt__DO_stmt_UNTIL_expr_OD(const tree* p);
void cg_stmt__READ_expr_SEMICOLON(const tree* p);
void cg_stmt__WRITE_expr_SEMICOLON(const tree* p);
void cg_stmt__expr_EQ_expr_SEMICOLON(const tree* p);
void cg_stmt__FREE_expr_SEMICOLON(const tree* p);
void cg_stmt__RETURN_SEMICOLON(const tree* p);
void cg_stmt__RETURN_expr_SEMICOLON(const tree* p);
void cg_stmtlist(const tree* p);
void cgx_call(const tree* p);
void cgx_coerce_stack_top_to_float( );
void cgx_coerce_stack_top_to_float_if_needed(const tree* need_type, const tree* have_type);
void cgx_coerce_stack_top_to_int( );
void cgx_common_externs( );
void cgx_construct_defn(const tree* p);
void cgx_construct_defnlist(const tree* p);
void cgx_construct_variable(const tree* p);
string cgx_define_array_constant(const tree* p);
string cgx_define_string_constant(const tree* leaf);
void cgx_delayed_functions( );
void cgx_destruct_defn(const tree* p);
void cgx_destruct_defnlist(const tree* p);
void cgx_destruct_variable(const tree* p);
void cgx_flop(const tree* p, string ffop, string fiop, string ifop);
void cgx_jump_for_false_boolexpr(const tree* p, int j);
void cgx_jump_for_false_compare(const tree* p, int label_number);
void cgx_jump_for_true_boolexpr(const tree* p, int j);
void cgx_jump_for_true_compare(const tree* p, int label_number);
void cgx_make_deep_copy(const tree* p);
void cgx_pop_to_variable(const tree* leaf);
void cgx_push_default(const tree* type);
void cgx_push_lval_expr(const tree* p);
void cgx_push_lval_expr__IDENTIFIER(const tree* p);
void cgx_push_lval_expr__STAR_expr(const tree* p);
void cgx_push_lval_expr__expr_LSQUARE_expr_RSQUARE(const tree* p);
void cgx_push_lval_expr__MINUSMINUS_expr(const tree* p);
void cgx_push_lval_expr__PLUSPLUS_expr(const tree* p);
void cgx_push_rval_expr(const tree* leaf);
void cgx_push_rval_expr__INTEGERVALUE(const tree* p);
void cgx_push_rval_expr__FLOATVALUE(const tree* p);
void cgx_push_rval_expr__STRINGVALUE(const tree* p);
void cgx_push_rval_expr__IDENTIFIER(const tree* p);
void cgx_push_rval_expr__IDENTIFIER_LPAREN_exprseq_RPAREN(const tree* p);
void cgx_push_rval_expr__LPAREN_ARRAY_OF_typeexpr_IS_exprseq_RPAREN(const tree* p);
void cgx_push_rval_expr__LPAREN_NEW_typeexpr_IS_expr_RPAREN(const tree* p);
void cgx_push_rval_expr__expr_OR_expr(const tree* p);
void cgx_push_rval_expr__expr_AND_expr(const tree* p);
void cgx_push_rval_expr__expr_COMPARE_expr(const tree* p);
void cgx_push_rval_expr__expr_HAT_expr(const tree* p);
void cgx_push_rval_expr__expr_MINUS_expr(const tree* p);
void cgx_push_rval_expr__expr_MINUSMINUS(const tree* p);
void cgx_push_rval_expr__expr_PERCENT_expr(const tree* p);
void cgx_push_rval_expr__expr_PLUS_expr(const tree* p);
void cgx_push_rval_expr__expr_PLUSPLUS(const tree* p);
void cgx_push_rval_expr__expr_STAR_expr(const tree* p);
void cgx_push_rval_expr__expr_SLASH_expr(const tree* p);
void cgx_push_rval_expr__FLOATCAST_expr(const tree* p);
void cgx_push_rval_expr__MINUS_expr(const tree* p);
void cgx_push_rval_expr__MINUSMINUS_expr(const tree* p);
void cgx_push_rval_expr__PLUSPLUS_expr(const tree* p);
void cgx_push_rval_expr__ROUND_expr(const tree* p);
void cgx_push_rval_expr__STAR_expr(const tree* p);
void cgx_push_rval_expr__expr_LSQUARE_expr_RSQUARE(const tree* p);
void cgx_push_shallow_rval_expr(const tree* leaf);
void cgx_push_shallow_rval_expr__STRINGVALUE(const tree* p);
void cgx_push_shallow_rval_expr__IDENTIFIER(const tree* p);
void cgx_push_shallow_rval_expr__LPAREN_ARRAY_OF_typeexpr_IS_exprseq_RPAREN(const tree* p);
void cgx_push_shallow_rval_expr__STAR_expr(const tree* p);
void cgx_push_shallow_rval_expr__expr_LSQUARE_expr_RSQUARE(const tree* p);
void cgx_push_static_link(int depth);
void cgx_read(const tree* type);
void cgx_set_carry_flag_from_floats(const tree* p1, const tree* p2);
void cgx_set_compare_flags(const tree* p);
bool is_array(const tree* type);
bool is_complex_array(const tree* type);
bool is_defn_reference(const tree* defn);
bool is_simple_array(const tree* type);
bool is_string(const tree* type);
bool is_using_implicit_memory(const tree* type);
string jump_label(int j);
string jump_label(string j);
void print_instruction(string comment, string inst, string op1 = "", string op2 = "");
void print_instruction(string comment, string inst, int op1, string op2 = "");
int unique_number( );
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global constants:
const tree* const FLOAT_TYPE = tt_primitive("|float|");
const tree* const INTEGER_TYPE = tt_primitive("|int|");
const tree* const STRING_TYPE = tt_primitive("|string|");

// The queue of delayed function definitions:
queue<const tree*> delayed_queue;

// The depth of any variable definitions that we process:
int current_depth = 0;
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void codegen(const tree* p)
// Written by Michael Main (Feb 2, 2011)
// This is the full code generator function.  Call it with the root of the
// entire parse tree to write the assembly program to cout.
// Precondition: p, a complete parse tree for a cu program, has been
// created by the lexer/parser/decorator with no errors.
// Postcondition: A nasm assembly program for p has been written to cout.
{
    cout.setf(ios_base::left);
    check(p->attribute<lhs>("LHS") == program__, "compile");
    
    cout << "# ..........................................................\n";
    cout << "# This is assembly code generated by the CU compiler.\n";
    cout << "# To assemble xxx.s into an executable: "
         << ASM_COMMAND;
    cout << "# ..........................................................\n";
    cout << '\n' << endl;

    cgx_common_externs( );
    cg_program(p);
    cgx_delayed_functions( );
    
    cout << "# ..........................................................\n";
    cout << "# End of assembly code generated by the 3155 compiler.\n";
    cout << "# ..........................................................";
    cout << endl;
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void cd_funcdefn(const tree* p)
// Written by Michael Main (Feb 2, 2011)
// The pointer p must be a pointer to a <funcdefn> node.
// The function generates code for the entire function, including
// code to manage a new function's frame.
//
// The format of the full function's frame is like this
// (with smaller addresses UP):
//                                        |          ...                |
//                                        | SECOND LOCAL VARIABLE       |
// ebp - size of first local variable --> | FIRST LOCAL VARIABLE        |
//                                ebp --> | DYNAMIC LINK (caller's epb) |
//                            ebp + 4 --> | STATIC LINK (someone's ebp) |
//                            ebp + 8 --> | STATIC DEPTH                |
//                           ebp + 12 --> | RETURN ADDRESS (to caller)  |
//                           ebp + 16 --> | FIRST PARAMETER             |
//                                        |          ...                |
//   ebp + 16 + (many_parameters-1)*4 --> | FINAL PARAMETER             |
//       ebp + 16 + many_parameters*4 --> | SPACE FOR RETURN VALUE      |
//                                        |          ...                |
//                                        |_____________________________|
//
// This code is responsible only for the items starting with the static
// depth, since the other items are pushed on the stack by the function
// caller.
{
    check(
	p->attribute<lhs>("LHS") == funcdefn__,
	"cd_funcdefn"
	);

    string name;    // Name of the function
    int offset;     // A unique number that's the Offset attribute 
    int depth;      // The depth of the function definition
    int body_index; // Index of body in the definition (6 or 8)
    const tree* stmtlist;
    const tree* defnlist;
    
    // 0. Set information about the function, including the global
    // current_depth to indicate the current depth of any definitions
    // that we will run into:
    name = p->child(1)->label( );
    offset = p->child(1)->attribute<int>("Offset");
    depth = p->child(1)->attribute<int>("Depth");
    body_index = p->many_children( ) - 1;
    defnlist = p->child(body_index)->child(1);
    stmtlist = p->child(body_index)->child(2);
    current_depth = depth+1;
    
    // 1. The entry-point label:
    cout << "entry." << name << "." << offset << ":\n";
    if (depth == 0)
    {   // Additional label for external linking of global functions/procedures
	cout << "__" << name << ":\n";
    }

    // 2. Create the function's frame
    PUSH(depth, "Static depth");
    cgx_push_static_link(depth);
    PUSH("%ebp", "Dynamic link (the old ebp)");
    MOV("%esp", "%ebp", "Set ebp to base of new function's frame");
    ALLOCATE_STACK(defnlist->attribute<int>("Bytes"), "For local variables");
    
    // 3. Initialize any local variables:
    cgx_construct_defnlist(defnlist);
    
    // 4. Generate the code for the function:
    cg_stmtlist(stmtlist);

    // 5. The exit-point label:
    cout << "jump.exit." << name << "." << offset << ":\n";

    // 6. Destroy any local variables that use implicit heap-dynamic memory:
    cgx_destruct_defnlist(defnlist);
    
    // 7. Unwind the function's frame
    MOV("%ebp", "%esp", "Move stack pointer back down");
    POP("%ebp", "Restore ebp from dynamic link");
    RELEASE_STACK(8, "Release static link and static depth");

    // 8. Return to the caller:
    RET("Return to the caller");
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void cg_program(const tree* p)
// Written by Michael Main (Feb 2, 2011)
// The pointer p must be a pointer to the root of the entire parse tree.
// This function generates code for the entire program, including an
// entry point called _WinMain$16 or main.
{
    check(p->attribute<lhs>("LHS") == program__, "cg_program");

    // Reserve the memory for global variables.
    cout << "# ...........................................................\n";
    cout << "# Include some useful assembly functions.\n";
    cout << "  .include \"cu.lib.s\"" << endl;
    cout << "# ...........................................................\n";
    cout << endl;
    cout << "# ...........................................................\n";
    cout << "# Data section for globals and compiler requirements.\n";
    cout << "  .section .data\n";
    cout << "  compiler.globals: .rept "
	 << p->attribute<int>("Bytes")
	 << "\n  .byte 0\n  .endr\n";
    cout << "  compiler.globals.base: .long 0\n";           // globals -4 from here
    cout << "  compiler.false: .asciz \"false\"\n";         // "false" null-termin
    cout << "  compiler.true: .asciz \"true\"\n";           // "true" null-termin
    cout << "  compiler.booleaninput: .long 0\n";           // |string| for bool in
    cout << "  compiler.integerformat: .asciz \"%d\"\n";    // for scanf or printf
    cout << "  compiler.floatformat: .asciz \"%f\"\n";      // for scanf or printf
    cout << "  compiler.stringformat: .asciz \"%s\"\n";     // for printf only
    cout << "  compiler.toos: .long 0\n";                   // return value to OS
    cout << "  .section .text\n";
    cout << "# ...........................................................\n";
    cout << '\n' << endl;

    cout << "# ..........................................................\n";
    cout << "# Generate code for globals, and call main function." << endl;
    cout << ".globl main\n";
    cout << "main:\n";
    cout << ".globl _main\n";
    cout << "_main:\n";
    
    // Set up a fake function's frame for pre-main function.  Notice that we
    // save all general purpose registers for whoever called this program.
    cout << "  fninit\n";  // Initialize the FPU
    cout << "  pusha\n";  // Save registers for whoever called this program
    PUSH(0, "Static depth of pre-main program");
    PUSH(0, "NULL static link for pre-main program");
    PUSH("%ebp", "Save ebp of pre-main's caller");
    MOV("%esp", "%ebp", "Set ebp to base of pre-main act rec");

    // Initialize space used by global variables, including
    // the compiler.booleaninput global |string| variable, which
    // is the string that scanf uses when it reads a boolean value:
    cgx_push_default(STRING_TYPE);
    POP("(compiler.booleaninput)", "Space for reading a boolean value");
    cgx_construct_defnlist(p->child(0));

    // Allocate space for the real main function's return value, call that
    // function and save its return value.
    ALLOCATE_STACK(4, "Space for main's return value");
    CALL("__main", "Call real main function");
    POP("(compiler.toos)", "(compiler.toos)=main's return value");

    // Destroy any variables that use implicit heap-dynamic memory.
    // This isn't really needed since we don't bother to release the
    // implicit heap-dynamic memory used by global variables
    // but in the future the destructors might do other work.
    cgx_destruct_defnlist(p->child(0));
    
    // Unwind the fake function's frame and set eax to main's return value
    // before returning to the operating system.
    MOV("%ebp", "%esp", "Move stack pointer back down");
    POP("%ebp", "Restore caller's ebp");
    ADD(8, "%esp", "Release static link and static depth");
    cout << "  popa\n";  // Restore registers for whoever called this program
    MOV("(compiler.toos)", "%eax", "%eax = main's return value");
    RET("Return to operating sys");
    cout << "# ..........................................................\n";
    cout << '\n' << endl;
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void cg_stmt(const tree* p)
// Written by Michael Main (Feb 3, 2011)
// The pointer p must be a pointer to a <stmt> node.
// The function generates code for the statement.
{
    check(p->attribute<lhs>("LHS") == stmt__, "cg_stmt");
    switch(p->attribute<rhs>("RHS"))
    {
    case __READ_expr_SEMICOLON:
	cg_stmt__READ_expr_SEMICOLON(p);
	break;
    case __WRITE_expr_SEMICOLON:
	cg_stmt__WRITE_expr_SEMICOLON(p);
	break;
    case __expr_EQ_expr_SEMICOLON:
	cg_stmt__expr_EQ_expr_SEMICOLON(p);
	break;
    case __IF_expr_THEN_stmt_FI:
	cg_stmt__IF_expr_THEN_stmt_FI(p);
	break;
    case __IF_expr_THEN_stmt_ELSE_stmt_FI:
	cg_stmt__IF_expr_THEN_stmt_ELSE_stmt_FI(p);
	break;
    case __WHILE_expr_DO_stmt_OD:
      	cg_stmt__WHILE_expr_DO_stmt_OD(p);
	break;
    case __DO_stmt_UNTIL_expr_OD:
       	cg_stmt__DO_stmt_UNTIL_expr_OD(p);
	break;
    case __LCURLY_stmtlist_RCURLY:
	cg_stmtlist(p->child(1));
	break;
    case __RETURN_SEMICOLON:
	cg_stmt__RETURN_SEMICOLON(p);
	break;
    case __RETURN_expr_SEMICOLON:
	cg_stmt__RETURN_expr_SEMICOLON(p);
	break;
    case __FREE_expr_SEMICOLON:
	cg_stmt__FREE_expr_SEMICOLON(p);
	break;
    case __IDENTIFIER_LPAREN_exprseq_RPAREN_SEMICOLON:
	cgx_call(p);
	break;
    default:
	break;
    }
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// For each of the cg_stmt__... functions, the pointer p must point to a <stmt>
// node with a certain rhs. All of these functions generate code for the
// statement in that tree.

void cg_stmt__IF_expr_THEN_stmt_FI(const tree* p)
// Written by Michael Main (Feb 3, 2011)
{  
    check(
	p->attribute<rhs>("RHS") ==__IF_expr_THEN_stmt_FI,
	"cg_stmt__IF_expr_THEN_stmt_FI"
	);

    // To do for HW 6.
}

void cg_stmt__IF_expr_THEN_stmt_ELSE_stmt_FI(const tree* p)
// Written by Michael Main (Feb 3, 2011)
{  
    check(
	p->attribute<rhs>("RHS") ==__IF_expr_THEN_stmt_ELSE_stmt_FI,
	"cg_stmt__IF_expr_THEN_stmt_ELSE_stmt_FI"
	);

        // To do for HW 6.
}

void cg_stmt__WHILE_expr_DO_stmt_OD(const tree* p)
// Written by Michael Main (Feb 3, 2011)
{  
    check(
	p->attribute<rhs>("RHS") ==__WHILE_expr_DO_stmt_OD,
	"cg_stmt__WHILE_expr_DO_stmt_OD"
	);

        // To do for HW 6.
}

void cg_stmt__DO_stmt_UNTIL_expr_OD(const tree* p)
// Written by Michael Main (Feb 3, 2011)
{  
    check(
	p->attribute<rhs>("RHS") == __DO_stmt_UNTIL_expr_OD,
	"cg_stmt__DO_stmt_UNTIL_expr_OD"
	);

        // To do for HW 6.
}

void cg_stmt__READ_expr_SEMICOLON(const tree* p)
// Written by Michael Main (Feb 3, 2011)
{
    check(p->attribute<rhs>("RHS") == __READ_expr_SEMICOLON, "cg_stmt__READ_expr_SEMICOLON");

    cgx_push_lval_expr(p->child(1));
    cgx_read(p->child(1)->attribute<const tree*>("Type"));
}

void cg_stmt__WRITE_expr_SEMICOLON(const tree* p)
// Written by Michael Main (Feb 3, 2011)
{
    check(p->attribute<rhs>("RHS") == __WRITE_expr_SEMICOLON, "cg_stmt__WRITE_expr_SEMICOLON");
    const tree* type = p->child(1)->attribute<const tree*>("Type");
    int label1, label2;           // Two labels for jumps in boolean case
    bool is_address_available;    // Does the expr have an address?

    if (is_compat(INTEGER_TYPE, type))
    {   // Print an integer:
	cgx_push_rval_expr(p->child(1)); // Push printf's int parameter
        PUSH("$compiler.integerformat", "Push printf's format argument");
        CALL("printf", "Call printf");
        RELEASE_STACK(8, "Pop the printf parameters");
    }
    else if (is_compat(STRING_TYPE, type))
    {   // Print a string:
	// Note that the rval of a string is a pointer byte number 16 of a
	// string record, which is where the null-terminated string data starts.
	is_address_available =  p->child(1)->attribute<bool>("Addressable");
	if (is_address_available)
	    cgx_push_shallow_rval_expr(p->child(1)); // Shallow string
	else
	    cgx_push_rval_expr(p->child(1));
        PUSH("$compiler.stringformat", "Push printf's format argument");
        CALL("printf", "Call printf");
	if (!is_address_available)
	{
	    MOV("4(%esp)", "%eax", "%eax = lib.freerec's argument");
	    CALL("lib.freerec", "Free string's implicit memory");
	}
	RELEASE_STACK(8, "Pop printf's arguments");
    }
    else if (is_compat(FLOAT_TYPE, type))
    {   // Print a float. 
	// Push the 4-byte float value
	cgx_push_rval_expr(p->child(1));
	
	// Convert that 4-byte float to an 8-byte float for printf
	FLD("(%esp)", "Load a 4-byte float from top of stack");
	ALLOCATE_STACK(4, "4 more bytes, so an 8-byte float");
	FSTP8("(%esp)", "Store the 8-byte float on top of the stack");

	// Push the format argument, call printf, and clean up:
	PUSH("$compiler.floatformat", "Push printf's format argument");
	CALL("printf", "Call printf");
	RELEASE_STACK(12, "Pop the printf parameters");	
    }
    else
    {   // Print a boolean. Note that we don't bother with pushing the rvalue
	// onto the stack: We just use the rvalue to control a jump directly.
        label1 = unique_number( );
        label2 = unique_number( );
        cgx_jump_for_true_boolexpr(p->child(1), label1);
        PUSH("$compiler.false", "Push format parameter for false");
        JUMP("jmp", label2, "Do not print the true case");
        LABEL(label1);
        PUSH("$compiler.true", "Push format parameter for true");
        LABEL(label2);
        CALL("printf", "Call printf");
        RELEASE_STACK(4, "Pop the printf parameter");
    }
}

void cg_stmt__expr_EQ_expr_SEMICOLON(const tree* p)
// Written by Michael Main (Feb 3, 2011)
{
    check(p->attribute<rhs>("RHS") == __expr_EQ_expr_SEMICOLON, "cg_stmt_expr_EQ_expr_SEMICOLON");

    const tree* target = p->child(0);
    const tree* need_type = target->attribute<const tree*>("Type");
    const tree* have_type = p->child(2)->attribute<const tree*>("Type");

    cgx_push_rval_expr(p->child(2));
    cgx_coerce_stack_top_to_float_if_needed(need_type, have_type);
	
    if (is_using_implicit_memory(target->attribute<const tree*>("Type")))
    {
	cgx_push_lval_expr(target);
	MOV("(%esp)", "%eax", "%eax = ptr to array or string to free");
	MOV("(%eax)", "%eax", "%eax = array or string to free");
	CALL("lib.freerec", "Free old implicit memory");
	POP("%ebx", "Pop l-value for assignment to ebx");
	POP("(%ebx)", "Pop r-value for assignment into its destination");
    }
    else if (target->attribute<rhs>("RHS") == __IDENTIFIER)
    {   // More efficient code if the l-value is just an identifier
	cgx_pop_to_variable(target->child(0));
    }
    else
    {   // Push the l-value of the first expression and pop to there
	cgx_push_lval_expr(target);
	POP("%ebx", "Pop l-value for assignment to ebx");
	POP("(%ebx)", "Pop r-value for assignment into its destination");
    }
}

void cg_stmt__RETURN_SEMICOLON(const tree* p)
// Written by Michael Main (Feb 3, 2011)
{
    check(
	p->attribute<rhs>("RHS") == __RETURN_SEMICOLON,
	"cg_stmt__RETURN_SEMICOLON"
	);
    const tree* defn = p;
    char op[MAX_OPERAND];
    
    do
    {
	defn = defn->parent();
    }   while (defn->attribute<lhs>("LHS") != funcdefn__);
    
    // Jump to the exit code for this function
    sprintf(op, "exit.%s.%d", defn->child(1)->label().c_str(), defn->child(1)->attribute<int>("Offset")); 
    JUMP("jmp", op, "To function exit");
}

void cg_stmt__RETURN_expr_SEMICOLON(const tree* p)
// Written by Michael Main (Feb 3, 2011)
{
    check(
	p->attribute<rhs>("RHS") == __RETURN_expr_SEMICOLON,
	"cg_stmt__RETURN_expr_SEMICOLON"
	);
    const tree* defn = p;
    const tree* need_type;
    const tree* have_type;
    char op[MAX_OPERAND];
    
    do
    {
	defn = defn->parent();
    }   while (defn->attribute<lhs>("LHS") != funcdefn__);
    
    // Compute the return value:
    need_type = defn->child(6);
    have_type = p->child(1)->attribute<const tree*>("Type");
    cgx_push_rval_expr(p->child(1));
    cgx_coerce_stack_top_to_float_if_needed(need_type, have_type);

    // Pop the return value in the return location.
    sprintf(
	op,
	"%d(%%ebp)",
	16 + defn->child(3)->attribute<int>("Bytes")
	);
    POP(op, "Pop the return value");
    
    // Jump to the exit code for this function
    sprintf(op, "exit.%s.%d", defn->child(1)->label().c_str(), defn->child(1)->attribute<int>("Offset")); 
    JUMP("jmp", op, "To function exit");
}

void cg_stmt__FREE_expr_SEMICOLON(const tree* p)
// Written by Michael Main (Feb 3, 2011)
// This function generates code to release the dynamic variable that a
// pointer points to.  If that dynamic variable uses any implicit
// dynamic memory, then that memory is also released.
{
    check(p->attribute<rhs>("RHS") == __FREE_expr_SEMICOLON, "cg_stmt__FREE_expr_SEMICOLON");
    const tree* child1 = p->child(1);
    const tree* type;

    // Compute the type of the data that we are releasing:
    type = child1->attribute<const tree*>("Type");
    type = tt_minus(type, POINTER);

    // Push a pointer to the dynamic memory that we're releasing
    cgx_push_rval_expr(child1);

    // If this pointer points to a string or array, then release the
    // implicit dynamic memory that it is using, too.
    if (is_using_implicit_memory(type))
    {   // Release the implicit memory of the string or array
	MOV("(%esp)", "%eax", "%eax = pointer to string or array");
	MOV("(%eax)", "%eax", "%eax = string or array");
	CALL("lib.freerec", "Free implicit dynamic memory");
    }

    // Free the memory that the pointer points to:
    CALL("free", "Free memory that pointer points to");
    RELEASE_STACK(4, "Pop free's argument");
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void cg_stmtlist(const tree* p)
// Written by Michael Main (Feb 3, 2011)
// The pointer p must be a pointer to a <stmtlist> node.
// The function generates code for each statement in the list.
{
    check(p->attribute<lhs>("LHS") == stmtlist__, "cg_stmtlist");
    
    if (p->attribute<rhs>("RHS") == __EMPTY)
	return;
    cg_stmtlist(p->child(0));
    cg_stmt(p->child(1));
}   
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void cgx_call(const tree* p)
// Written by ...
// The pointer p must be a pointer to a <stmt> or <expr> node that is a
// function or procedure call.  cgx_call generates assembly code that
// calls the function or procedure.  If it is a function, the function's return
// value is left on top of the stack.
{
    check(
	p->attribute<rhs>("RHS") ==__IDENTIFIER_LPAREN_exprseq_RPAREN
	||
	p->attribute<rhs>("RHS") ==__IDENTIFIER_LPAREN_exprseq_RPAREN_SEMICOLON,
	"cgx_call"
	);

    // This is part of HW 7. Note: Without cgx_call, function calls in
    // the CU language will generate no assembly code.
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void cgx_coerce_stack_top_to_float( )
// Written by Michael Main (Feb 3, 2011)
// This function generates code so that
// the 4-byte int on top of the stack is coerced to a 4-byte float.
{
    FILD("(%esp)", "Load an int to float stack");
    FSTP("(%esp)", "Store back to run-time stack");
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void cgx_coerce_stack_top_to_float_if_needed(const tree* need_type, const tree* have_type)
// Precondition: Both arguments are type trees.
// Postcondition: If the need_type tree is float and the have_type tree is an
// int, then cgx_coerce_stack_top_to_float is called to generate code to coerce
// the 4-byte int on top of the stack to a 4-byte float.
// If the need_type is an array of float (possibly multidimensional), and the
// have_type is an array of int, then two instructions are generated to coerce
// the array on top of the stack from an array of int to an array of float:
// mov eax (%esp) // might be wrong syntax
// call lib.coercerec
{
    // To do for HW 5.
	
	// if need type is float and have_type is int:
	if( is_compat(need_type, FLOAT_TYPE, false) && is_compat(have_type, INTEGER_TYPE, false) ) { 
		cgx_coerce_stack_top_to_float();
	}
	else if ( format_of_tt(need_type) == ARRAY) {
		const tree * ltype = need_type;
		const tree * rtype = have_type;

		while ( format_of_tt(ltype) == ARRAY) {
			ltype = tt_minus(ltype, format_of_tt(ltype));
			rtype = tt_minus(rtype, format_of_tt(rtype));

			if ( is_compat(ltype, FLOAT_TYPE, false) && is_compat(rtype, INTEGER_TYPE, false) ){
				mov((%esp), %eax, "move eax onto stack");
				call(lib.coercerec, "call lib.coercerec");
			}
		}
		}
/*
// Some type functions from cu.traverser.cxx
extern int format_of_tt(const tree* tt);
extern bool is_compat(const tree* var_type, const tree* expr_type, bool allow_coercion = true);
extern int seq_length(const tree* p);
extern const tree* const tt_primitive(string primitive); 
extern const tree* tt_minus(const tree* tt, int qualifier);
*/



}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void cgx_coerce_stack_top_to_int( )
// Written by Michael Main (Feb 3, 2011)
// This function generates code so that
// the 4-byte float on top of the stack is coerced to a 4-byte int.
{
    FLD("(%esp)", "Load the float to the float stack");
    FISTPL("(%esp)", "Store back to run-time stack as int");
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void cgx_construct_defn(const tree* p)
// Written by Michael Main (Feb 3, 2011)
// The pointer p must be a pointer to a <vardefn> or <funcdefn> node.
// Variable definitions will generate code to initialize the variable.
// Function definitions are pushed onto a stack to have its code generated
// later.
{
    check(p->attribute<lhs>("LHS") == defn__, "cgx_construct_defn");
    switch(p->attribute<rhs>("RHS"))
    {
    case __vardefn:
	cgx_construct_variable(p->child(0));
	break;
    case __funcdefn:
	delayed_queue.push(p->child(0));
	break;
    default:
	break;
    }
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void cgx_construct_defnlist(const tree* p)
// Written by Michael Main (Feb 3, 2011)
// The pointer p must be a pointer to a <defnlist> node. The function generates
// code to initialize every variable in the definition list.  Each function
// definition in the list is pushed onto a stack to have its code generated
// later.
{
    check(p->attribute<lhs>("LHS") == defnlist__, "cg_construct_defnlist");

    if (p->attribute<rhs>("RHS") == __EMPTY)
        return;
    cgx_construct_defnlist(p->child(0));
    cgx_construct_defn(p->child(1));
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void cgx_construct_variable(const tree* p)
// Written by Michael Main (Feb 3, 2011)
// Testing: cu-test-programs/test007-defaults.cu
// Code review: with Paul Levin (Feb 3, 2011)
// The pointer p must be a pointer to a <vardefn>, <globaldefn> or
// <staticdefn>. This function generates code to initialize the variable.
{
    check(
	p->attribute<lhs>("LHS") == vardefn__,
	"cg_construct_variabledefn"
	);
    const tree* type = p->child(0);
    const tree* have_type;
    
    // Push the default value onto the stack.
    if (p->many_children( ) == 3)
    {   // No initialization value, so use the default
	cgx_push_default(type);
    }
    else
    {   // Push the r-value of the initialization expression
	cgx_push_rval_expr(p->child(4));
	have_type = p->child(4)->attribute<const tree*>("Type");
	cgx_coerce_stack_top_to_float_if_needed(type, have_type);
    }

    // Pop the value that we just calculated to the variable.
    cgx_pop_to_variable(p->child(1));
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// void cgx_ common_externs( )
// Written by Michael Main (Sep 25, 2008)
// This function generates the code needed to link to various c library
// functions using the names that are common for Windows and Linux. It
// also includes helpful functions from cu.lib.s.
void cgx_common_externs( )
{
#ifdef __MINGW_H // Windows
    cout << "# ..........................................................\n";
    cout << "# Allow each external to be accessed via its Windows name\n";
    cout << "  .set main,_main" << endl;
    cout << "  .set calloc,_calloc" << endl;
    cout << "  .set free,_free" << endl;
    cout << "  .set getchar,_getchar" << endl;
    cout << "  .set malloc,_malloc" << endl;
    cout << "  .set memcpy,_memcpy" << endl;
    cout << "  .set pow,_pow" << endl;
    cout << "  .set printf,_printf" << endl;
    cout << "  .set realloc,_realloc" << endl;
    cout << "  .set scanf,_scanf"<< endl;
    cout << "  .set stdin,__imp___iob" << endl;
    cout << "  .set strcat,_strcat" << endl;
    cout << "  .set strcmp,_strcmp" << endl;
    cout << "  .set strcpy,_strcpy" << endl;
    cout << "  .set strlen,_strlen" << endl;
    cout << "  .set ungetc,_ungetc" << endl;
    cout << "# ..........................................................\n";
    cout << '\n' << endl;
#endif
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
string cgx_define_array_constant(const tree* p)
// Written by Michael Main (Feb 3, 2011)
// Generates code to create a readonly array of readonly elements in
// static memory.  The return value is a string that can be used
// in an assembly instruction as the r-value of the static array
// (a pointer to byte 16 of the actual array).
{
    char record_name[MAX_OPERAND];
    char destination[MAX_OPERAND];
    size_t i, many, index;
    const tree* pal;
    const tree* pex;
    const tree* ptmp;
    const tree* need_type = p->child(3);
    const tree* have_type;
    
    // Create an array record in the data section.  The format of the record
    // is described in the data type comments at the top of this file.
    sprintf(record_name, "arrayrecord.%d", unique_number( ));
    pal = ptmp = p->child(5);

    // Count how many elements in the array:
    many = 0;
    while (ptmp->many_children( ) == 3)
    {
	++many;
	ptmp = ptmp->child(0);
    }
    if (ptmp->many_children( ) != 0)
	++many;
    
    cout << "\n  .section .data\n";
    LONG(16+many*4, "Start of an array record (total bytes)");
    LONG((is_using_implicit_memory(p->child(3))?1:0), "What kind of array");
    LONG(0, "Reserved for future use");
    LONG(many, "Current size of the array record");
    cout << "  " << record_name << ":\n  .rept " << many*4 << "\n  .byte 0\n  .endr\n";
    cout << "\n  .section .text\n" << endl;
    for (i = 0; i < many; ++i)
    {
	// Set pex to pal's <expr> child.  The index for this child is many - i - 1;
	pex = pal->child(pal->many_children( )-1);
	index = many - i - 1;
	cgx_push_rval_expr(pex);
	have_type = pex->attribute<const tree*>("Type");
	cgx_coerce_stack_top_to_float_if_needed(need_type, have_type);
	sprintf(destination, "(%s+%lu)", record_name, index*4);
	POP(destination, "Pop an array component");
	pal = pal->child(0); // Down to the next exprseq
    }

    return record_name;
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
string cgx_define_string_constant(const tree* leaf)
// Written by Michael Main (Feb 3, 2011
{
    const string value = leaf->label( );
    char record_name[MAX_OPERAND];
    size_t i, length;

    // Create a string-record in the data section.  The format of the record
    // is described in the data type comments at the top of this file.
    sprintf(record_name, "stringrecord.%d", unique_number( ));
    cout << "\n  .section .data\n";
    length = 0;
    for (i = 1; i < value.size( )-1; ++i)
    {
	++length;
        if (value[i] == '\\')
            ++i;
    }
    LONG(length+17, "Length of a string record");
    LONG(-1, "What kind of record? (-1 is string)");
    LONG(length, "Maximum chars in the string record");
    LONG(length, "Current chars in the string record");
    cout << "  " << record_name << ": .byte ";
    for (i = 1; i < value.size( )-1; ++i)
    {
        if (value[i] == '\\')
        {
            ++i;
            if (isdigit(value[i]))
            {
                cout << value[i++];
                if (isdigit(value[i]))
                    cout << value[i++];
                if (isdigit(value[i]))
                    cout << value[i++];
            }
            else switch(value[i])
            {
            case 'n': cout << int('\n'); break;
            case 't': cout << int('\t'); break;
            case 'v': cout << int('\v'); break;
            case 'b': cout << int('\b'); break;
            case 'r': cout << int('\r'); break;
            case 'f': cout << int('\f'); break;
            case 'a': cout << int('\a'); break;
            case '\\': cout << int('\\'); break;
            case '?': cout << int('\?'); break;
            case '\'': cout << int('\''); break;
            case '"': cout << int('\"'); break;
            default: cout << int(value[i]); break;
            }
        }
        else
            cout << int(value[i]);
        cout << ',';
    }
    cout << "0\n";
    cout << "  .section .text\n" << endl;

    return record_name;
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void cgx_delayed_functions( )
// Written by Michael Main (Feb 3, 2011)
// This function causes all the definitions in delayed_queue
// to have their code generated.
{
    while (!delayed_queue.empty( ))
    {   // Generate the code for the function definition at the front of
	// the queue.  Notice that cd_funcdefn could add more
	// function definitions to the queue.
	cout << "# ..........................................................\n";
	cd_funcdefn(delayed_queue.front( ));
	delayed_queue.pop( );
	cout << "# ..........................................................\n";
	cout << '\n' << endl;
    }
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void cgx_destruct_defn(const tree* p)
// Written by Michael Main (Feb 3, 2011)
// The pointer p must be a pointer to a <vardefn> node.
// If it is a <vardefn>, this functiion generates code to release the
// variable's implicit heap-dynamic memory.
{
    check(p->attribute<lhs>("LHS") == defn__, "cg_destruct_defn");
    switch(p->attribute<rhs>("RHS"))
    {
    case __vardefn:
	cgx_destruct_variable(p->child(0));
	break;
    default:
	break;
    }
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void cgx_destruct_defnlist(const tree* p)
// Written by Michael Main (Feb 3, 2011)
// Testing: cu-test-programs/test007-defaults.cu
// Code review: with Barbara Radley-Wittenham (Feb 3, 2011)
// The pointer p must be a pointer to a <defnlist> node. The function generates
// code to cleanup every variable in the definition list.  This requires work
// only for strings and arrays.
// later.
{
    check(p->attribute<lhs>("LHS") == defnlist__, "cg_destruct_defnlist");

    if (p->attribute<rhs>("RHS") == __EMPTY)
        return;
    cgx_destruct_defn(p->child(1));
    cgx_destruct_defnlist(p->child(0));
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void cgx_destruct_variable(const tree* p)
// Written by Michael Main (Feb 3, 2011)
// The pointer p must be a pointer to a <vardefn> node.
// If p is a string or array, then this function generates code to release
// any implicit heap-dynamic memory used by the variable.
{
    check(p->attribute<lhs>("LHS") == vardefn__, "cg_destruct_variabledefn");
    const tree* type = p->child(0);
    char op[MAX_OPERAND];
    int offset = p->child(1)->attribute<int>("Offset");
    int identifier_depth = p->child(1)->attribute<int>("Depth");
    
    if (is_using_implicit_memory(type))
    {   // Free the implicit heap dynamic memory used by this variable
	if (identifier_depth == 0)
	    sprintf(op, "(compiler.globals.base+%d)", offset);
	else
	    sprintf(op, "%d(%%ebp)", offset);
	MOV(op, "%eax", "%eax = rvalue of array or string");
	CALL("lib.freerec", "Free implicit dynamic memory");
    }
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void cgx_flop(
    const tree* p,
    string ffop, string fiop, string ifop,
    bool comparison
    )
// Written by Michael Main (Feb 3, 2011)
// Precondition: p is an expr node for a binary expression
// with at least one of the two operands being a float.
// Postcondition: The code to evaluate the arithmetic expression has been
// generated as follows:
// If both arguments were float then: st0 = op1; (%esp) = op2; ffop.
// If the 1st is a float then: st0 = op1; (%esp) = op2; fiop.
// If the 2nd is a float then: st0 = op2; (%esp) = op1; ifop.
// The (%esp) op is then removed from the run-time stack.
// The result of the arithmetic operation is popped from the float stack
// and pushed onto the run-time stack as a 4-byte float.
{
    check(p->attribute<lhs>("LHS") == expr__, "cgx_flop");
    const tree* type1 = p->child(0)->attribute<const tree*>("Type");
    const tree* type2 = p->child(2)->attribute<const tree*>("Type");

    if (is_compat(INTEGER_TYPE, type1))
    {   // Use ifop
	cgx_push_rval_expr(p->child(0));
	cgx_push_rval_expr(p->child(2));
	FLD("(%esp)", "st0 = op2 for flop");
	RELEASE_STACK(4, "Release memory used by op2");
	cout << "  " << ifop 
	     << setw(TAB-10-ifop.length( )) << " (%esp)" << " # "
	     << "float-integer op" << endl;
    }
    else if (is_compat(INTEGER_TYPE, type2))
    {   // Use fiop 
	cgx_push_rval_expr(p->child(2));
	cgx_push_rval_expr(p->child(0));
	FLD("(%esp)", "st0 = op1 for flop");
	RELEASE_STACK(4, "Release memory used by op1");
	cout << "  " << fiop 
	     << setw(TAB-10-fiop.length( )) << " (%esp)" << " # "
	     << "float-intger op" << endl;
    }
    else
    {   // Use ffop
	cgx_push_rval_expr(p->child(2));
	cgx_push_rval_expr(p->child(0));
	FLD("(%esp)", "st0 = op1 for flop");
	RELEASE_STACK(4, "Release memory used by op1");
	cout << "  " << ffop 
	     << setw(TAB-10-ffop.length( )) << " (%esp)" << " # "
	     << "float-float op" << endl;
    }
    FSTP("(%esp)", "Put result back on stack");
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void cgx_jump_for_false_boolexpr(const tree* p, int label_number)
// Written by Michael Main (Feb 3, 2011)
// The pointer p must be a pointer to an <expr> node that is used as a
// boolean expression in some statement.
// This function generates assembly code that will evaluate the boolean
// expression and jump to the label_number if the expression is FALSE.
// Otherwise, no jump occurs.  In either case, the stack is left unchanged.
// In particular: The value of the boolean expression is not left on top
// of the stack.
{
    check(p->attribute<lhs>("LHS") == expr__, "cgx_jump_for_false_boolexpr");
    int other_number;
    
    switch(p->attribute<rhs>("RHS"))
    {
    case __TRUE:
	break;
    case __FALSE:
            JUMP("jmp", label_number, "Jump for false constant");
        break;
    case __expr_AND_expr:
        cgx_jump_for_false_boolexpr(p->child(0), label_number);
        cgx_jump_for_false_boolexpr(p->child(2), label_number);
        break;
    case __expr_OR_expr:
        other_number = unique_number( );
        cgx_jump_for_true_boolexpr(p->child(0), other_number);
        cgx_jump_for_false_boolexpr(p->child(2), label_number);
        LABEL(other_number);
        break;
    case __NOT_expr:
        cgx_jump_for_true_boolexpr(p->child(1), label_number);
        break;
    case __expr_LT_expr:
    case __expr_GT_expr:
    case __expr_LE_expr:
    case __expr_GE_expr:
    case __expr_EQEQ_expr:
    case __expr_NE_expr:
        cgx_jump_for_false_compare(p, label_number);
        break;
    case __LPAREN_expr_RPAREN:
        cgx_jump_for_false_boolexpr(p->child(1), label_number);
        break;
    case __IDENTIFIER:
    case __IDENTIFIER_LPAREN_exprseq_RPAREN:
    case __expr_LSQUARE_expr_RSQUARE:
    case __STAR_expr:
        cgx_push_rval_expr(p);
        POP("%eax", "Pop value of boolean variable");
        CMP(0, "%eax", "Check value of boolean variable");
        JUMP("je", label_number, "Jump if false");
        break;
    default:
        check(false, "cgx_jump_for_false_boolexpr");
	break;
    }
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void cgx_jump_for_false_compare(const tree* p, int label_number)
// Written by Michael Main (Feb 3, 2011)
// The pointer p must be a pointer to an <expr> node that is used as a
// boolean comparison in some statement.
// This function generates assembly code that will evaluate the boolean
// expression and jump to the label_number if the expression is FALSE.
// Otherwise, no jump occurs.  In either case, the stack is left unchanged.
// In particular: The value of the boolean expression is not left on top
// of the stack.
{   
    check(p->attribute<lhs>("LHS") == expr__, "cgx_jump_for_false_compare");
    const tree* type1 = p->child(0)->attribute<const tree*>("Type");
    const tree* type2 = p->child(2)->attribute<const tree*>("Type");
    bool is_float = !is_compat(INTEGER_TYPE, type1) || !is_compat(INTEGER_TYPE, type2);

    if (is_float)
    {   // At least one is a float
	switch(p->attribute<rhs>("RHS"))
	{
        case __expr_LT_expr:
	    cgx_set_carry_flag_from_floats(p->child(0), p->child(2));
	    JUMP("jae", label_number, "Jump if !(a < b)");
	    break;
	case __expr_GT_expr:
	    cgx_set_carry_flag_from_floats(p->child(2), p->child(0));
	    JUMP("jae", label_number, "Jump if !(a > b)");
	    break;
	case __expr_LE_expr:
	    cgx_set_carry_flag_from_floats(p->child(0), p->child(2));
	    JUMP("ja", label_number, "Jump if !(a <= b)");
	    break;
	case __expr_GE_expr:
	    cgx_set_carry_flag_from_floats(p->child(2), p->child(0));
	    JUMP("ja", label_number, "Jump if !(a >= b)");
	    break;
	case __expr_EQEQ_expr:
	    cgx_set_carry_flag_from_floats(p->child(0), p->child(2));
	    JUMP("jne", label_number, "Jump if !(a == b)");
	    break;
	case __expr_NE_expr:
	    cgx_set_carry_flag_from_floats(p->child(0), p->child(2));
	    JUMP("je", label_number, "Jump if !(a != b)");
	    break;
	default:
	    check(false, "cgx_jump_for_false_compare");
	    break;
	}
    }
    else
    {   // Both are int
	cgx_set_compare_flags(p);
	switch(p->attribute<rhs>("RHS"))
	{
	case __expr_LT_expr:
	    JUMP("jge", label_number, "Jump for !(expr1 < expr2)");
	    break;
	case __expr_GT_expr:
	    JUMP("jle", label_number, "Jump for !(expr1 > expr2)");
	    break;
	case __expr_LE_expr:
	    JUMP("jg", label_number, "Jump for !(expr1 <= expr2)");
	    break;
	case __expr_GE_expr:
	    JUMP("jl", label_number, "Jump for !(expr1 >= expr2)");
	    break;
	case __expr_EQEQ_expr:
	    JUMP("jne", label_number, "Jump for !(expr1 == expr2)");
	    break;
	case __expr_NE_expr:
	    JUMP("je", label_number, "Jump for !(expr1 != expr2)");
	    break;
	default:
	    check(false, "cgx_jump_for_false_compare");
	    break;
	}
    }
}
//-----------------------------------------------------------------------------

    
//-----------------------------------------------------------------------------
void cgx_jump_for_true_boolexpr(const tree* p, int label_number)
// Written by Michael Main (Feb 3, 2011)
// The pointer p must be a pointer to an <expr> node that is used as a
// boolean expression in some statement.
// This function generates assembly code that will evaluate the boolean
// expression and jump to the label_number if the expression is TRUE.
// Otherwise, no jump occurs.  In either case, the stack is left unchanged.
// In particular: The value of the boolean expression is not left on top
// of the stack.
{
    check(p->attribute<lhs>("LHS") == expr__, "cgx_jump_for_true_boolexpr");
    int other_number;

    switch(p->attribute<rhs>("RHS"))
    {
    case __TRUE:
	JUMP("jmp", label_number, "Jump for true constant");
        break;
    case __FALSE:
	break;
    case __expr_AND_expr:
        other_number = unique_number( );
        cgx_jump_for_false_boolexpr(p->child(0), other_number);
        cgx_jump_for_true_boolexpr(p->child(2), label_number);
        LABEL(other_number);
        break;
    case __expr_OR_expr:
        cgx_jump_for_true_boolexpr(p->child(0), label_number);
        cgx_jump_for_true_boolexpr(p->child(2), label_number);
        break;
    case __NOT_expr:
        cgx_jump_for_false_boolexpr(p->child(1), label_number);
        break;
    case __expr_LT_expr:
    case __expr_GT_expr:
    case __expr_LE_expr:
    case __expr_GE_expr:
    case __expr_EQEQ_expr:
    case __expr_NE_expr:
        cgx_jump_for_true_compare(p, label_number);
        break;
    case __LPAREN_expr_RPAREN:
        cgx_jump_for_true_boolexpr(p->child(1), label_number);
        break;
    case __IDENTIFIER_LPAREN_exprseq_RPAREN:
    case __expr_LSQUARE_expr_RSQUARE:
    case __STAR_expr:
    case __IDENTIFIER:
	cgx_push_rval_expr(p);
        POP("%eax", "Pop value of boolean variable");
        CMP(0, "%eax", "Check value of boolean variable");
        JUMP("jne", label_number, "Jump if true");
        break;
    default:
        check(false, "cgx_jump_for_true_boolexpr");
	break;
    }
}
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
void cgx_jump_for_true_compare(const tree* p, int label_number)
// Written by Michael Main (Feb 3, 2011)
// The pointer p must be a pointer to an <expr> node that is used as a
// boolean comparison in some statement.
// This function generates assembly code that will evaluate the boolean
// expression and jump to the label_number if the expression is TRUE.
// Otherwise, no jump occurs.  In either case, the stack is left unchanged.
// In particular: The value of the boolean expression is not left on top
// of the stack.
{
    check(p->attribute<lhs>("LHS") == expr__, "cgx_jump_for_true_compare");
    const tree* type1 = p->child(0)->attribute<const tree*>("Type");
    const tree* type2 = p->child(2)->attribute<const tree*>("Type");
    bool is_float = !is_compat(INTEGER_TYPE, type1) || !is_compat(INTEGER_TYPE, type2);

    if (is_float)
    {   // At least one is a float
	switch(p->attribute<rhs>("RHS"))
	{
        case __expr_LT_expr:
	    cgx_set_carry_flag_from_floats(p->child(2), p->child(0));
	    JUMP("ja", label_number, "");
	    break;
	case __expr_GT_expr:
	    cgx_set_carry_flag_from_floats(p->child(0), p->child(2));
	    JUMP("ja", label_number, "");
	    break;
	case __expr_LE_expr:
	    cgx_set_carry_flag_from_floats(p->child(2), p->child(0));
	    JUMP("jae", label_number, "");
	    break;
	case __expr_GE_expr:
	    cgx_set_carry_flag_from_floats(p->child(0), p->child(2));
	    JUMP("jae", label_number, "");
	    break;
	case __expr_EQEQ_expr:
	    cgx_set_carry_flag_from_floats(p->child(0), p->child(2));
	    JUMP("je", label_number, "");
	    break;
	case __expr_NE_expr:
	    cgx_set_carry_flag_from_floats(p->child(0), p->child(2));
	    JUMP("jne", label_number, "");
	    break;
	default:
	    check(false, "cgx_jump_for_false_compare");
	    break;
	}
    }
    else
    {   // Both are int
	cgx_set_compare_flags(p);

	switch(p->attribute<rhs>("RHS"))
	{   
	case __expr_LT_expr:
	    JUMP("jl", label_number, "Jump for (expr1 < expr2)");
	    break;
	case __expr_GT_expr:
	    JUMP("jg", label_number, "Jump for (expr1 > expr2)");
	    break;
	case __expr_LE_expr:
	    JUMP("jle", label_number, "Jump for (expr1 <= expr2)");
	    break;
	case __expr_GE_expr:
	    JUMP("jge", label_number, "Jump for (expr1 >= expr2)");
	    break;
	case __expr_EQEQ_expr:
	    JUMP("je", label_number, "Jump for (expr1 == expr2)");
	    break;
	case __expr_NE_expr:
	    JUMP("jne", label_number, "Jump for (expr1 != expr2)");
	    break;
	default:
	    check(false, "cgx_jump_for_true_compare");
	    break;
	}
    }        
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void cgx_make_deep_copy(const tree* p)
// Written by Michael Main (Feb 3, 2011)
// When this function is called, (%esp) is a pointer to an array or string
// record (as described in the documentation of cgx_push_rval_expr).
// This function generates code to make a copy of that array or string.
// The value in (%esp) is replaced with a pointer to this new deep copy.
// The original array or string record still exists, unaltered.
{
    MOV("%esp", "%eax", "%eax = pointer to the array or string on stack top");
    CALL("lib.copyrec", "(%esp) = new deep copy of that array or string");
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void cgx_pop_to_variable(const tree* leaf)
// Written by Michael Main (Feb 3, 2011)
// The parameter, leaf, is an IDENTIFER token for a variable with a type that
// is not a string or an array. This function generates somewhat efficient code
// to pop the value on top of the stack to that identifier.
// (See cg_stmt__expr_EQ_expr_SEMICOLON for the less efficient code that works
// for arrays and strings).
{
    check(leaf->attribute<int>("Token") == IDENTIFIER, "cgx_pop_to_variable");

    char op[MAX_OPERAND];
    int identifier_depth = leaf->attribute<int>("Depth");
    int offset = leaf->attribute<int>("Offset");
    int distance = current_depth - identifier_depth;
    
    // Always pop to the variable's l-value.
    // We start by putting that address into a string, op.
    if (identifier_depth == 0)
    {   // Pop to global variable
        sprintf(op, "(compiler.globals.base+%d)", offset);
    }
    else if (distance == 0)
    {   // Pop to a local variable or parameter of the current frame
	// Part of HW 7. Without this part of the code generator,
	// only global variables will work in a CU program.
    }
    else
    {   // A local variable or parameter in a distant frame
	// Part of HW 7. Without this part of the code generator,
	// only global variables will work in a CU program.
    }

    POP(op, "Pop to variable");
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void cgx_push_default(const tree* type)
// Written by Michael Main (Feb 3, 2011)
{
    check(type->attribute<lhs>("LHS") == typeexpr__, "cgx_push_default");

    if (is_compat(type, FLOAT_TYPE))
    {   // Initialize a float with 0 constant:
	FLDZ("Default value for float");
    }
    else if (!is_using_implicit_memory(type))
    {   // An |int|, |bool|, or pointer to something.
	PUSH(0, "Default value");
    }
    else if (is_string(type))
    {   // A string. Make space for 15 characters initially, and
	// set the string equal to the empty string initially.
	PUSH(1, "Push calloc's elemsize argument");
	PUSH(32, "Push calloc's numelem argument");
	CALL("calloc", "%eax = memory for new string");
	RELEASE_STACK(8, "Pop calloc's arguments");
	MOV(32, "(%eax)", "Number of bytes in the record");
	MOV(-1, "4(%eax)", "Indicates the type is a string");
	MOV(15, "8(%eax)", "Maximum number of characters");
	// 12(%eax) (current number of chars) is already zero from calloc
	// 16(%eax) (null-terminate string) is already zero from calloc
	ADD(16, "%eax", "%eax = pointer to data area");
	PUSH("%eax", "Push string rvalue");
    }
    else
    {   // An empty array.
	// Note: The language does not yet allow the programmer to allocate
	// an initial size for the array.
	PUSH(16, "Push malloc's size argument");
	CALL("malloc", "%eax = memory for new array");
	RELEASE_STACK(4, "Pop malloc's arguments");
	MOV(16, "(%eax)", "Number of bytes in the record");
	if (is_simple_array(type))
	    MOV(0, "4(%eax)", "Array type");
	else
	    MOV(1, "4(%eax)", "Array type");
	// 8(%eax) is reserved for future use
	MOV(0, "12(%eax)", "Current number of elements");
	ADD(16, "%eax", "%eax = pointer to (empty) data area");
	PUSH("%eax", "Push string rvalue");
    }
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void cgx_push_lval_expr(const tree* p)
// Written by Michael Main (Feb 3, 2011)
// The pointer p must point to an <expr> node.  This function generates code
// that evaluates the expression and leaves the lvalue of that expression on
// top of the stack.  This is always a 4-byte address.
{
    check(p->attribute<lhs>("LHS") == expr__, "cgx_push_lval_expr");

    switch(p->attribute<rhs>("RHS"))
    {
    case __IDENTIFIER:
        cgx_push_lval_expr__IDENTIFIER(p);
	break;
    case __LPAREN_expr_RPAREN:
        cgx_push_lval_expr(p->child(1));
	break;
    case __STAR_expr:
	cgx_push_lval_expr__STAR_expr(p);
	break;
    case __expr_LSQUARE_expr_RSQUARE:
	// This is the l-value of a component of an array
	cgx_push_lval_expr__expr_LSQUARE_expr_RSQUARE(p);
	break;
    case __MINUSMINUS_expr:
	cgx_push_lval_expr__MINUSMINUS_expr(p);
	break;
    case __PLUSPLUS_expr:
	cgx_push_lval_expr__PLUSPLUS_expr(p);
	break;
    default:
        check(false, "cgx_push_lval_expr");
	break;
    }
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// For each of the cgx_push_lval_expr__... functions, the pointer p must point
// to an <expr> node with a certain rhs. All of these functions generate code
// to leave the l-value of the expression on top of the stack.
// (which is always a 4 byte address).
void cgx_push_lval_expr__IDENTIFIER(const tree* p)
// Written by Michael Main (Feb 3, 2011)
{
    check(p->attribute<rhs>("RHS") == __IDENTIFIER, "cgx_push_lval_expr__IDENTIFIER");
    char op[MAX_OPERAND];
    const tree* leaf = p->child(0);
    int identifier_depth = leaf->attribute<int>("Depth");
    int offset = leaf->attribute<int>("Offset");
    int distance = current_depth - identifier_depth;
    
    if (identifier_depth == 0)
    {   // Global variable
	sprintf(op, "$compiler.globals.base+(%d)", offset);
	PUSH(op, "Address of global reference variable");
    }
    else if (distance == 0)
    {   // A local variable or parameter of the current function's frame
	// Part of HW 7. Without this part of the code generator,
	// only global variables will work in a CU program.
    }
    else
    {   // A variable or parameter in a distant function's frame
	// Part of HW 7. Without this part of the code generator,
	// only global variables will work in a CU program.
    }
}

void cgx_push_lval_expr__STAR_expr(const tree* p)
// Written by Michael Main (Feb 3, 2011)
{
    check(
	p->attribute<rhs>("RHS") == __STAR_expr,
	"cgx_push_lval_expr__STAR_expr"
	);

    cgx_push_rval_expr(p->child(1));
}

void cgx_push_lval_expr__expr_LSQUARE_expr_RSQUARE(const tree* p)
// Written by Michael Main (Feb 3, 2011)
{
    check(
	p->attribute<rhs>("RHS") ==__expr_LSQUARE_expr_RSQUARE,
	"cgx_push_lval_expr__expr_LSQUARE_expr_RSQUARE"
	);
    int label = unique_number( );
    int label_loop_top = unique_number( );
    
    // Note: In order to get the l-value of an array component, we require
    // that the array itself must have an l-value.  Otherwise, we cannot
    // increase its size if needed.  The decorator ensures this.
    cgx_push_lval_expr(p->child(0));  // The array's l-value
    cgx_push_rval_expr(p->child(2));  // The index
    POP("%ebx", "%ebx = index of the array element");
    POP("%ecx", "%ecx = l-value of the array variable");
    MOV("(%ecx)", "%eax", "pointer to byte 16 of the array record");
    CMP("-4(%eax)", "%ebx", "Is index < array size?");
    JUMP("jl", label, "If so, then jump over realloc");

    // Resize the array so that it has the specified index.
    // NOTE: Later we should increase beyond this amount to prevent
    // continual resizing.
    PUSH("%ecx", "Save the ecx register");
    PUSH("%ebx", "Push the index that we want");
    SHL(2, "(%esp)", "New number of bytes is 4*index...");
    ADD(20, "%esp)", "...plus 20");
    PUSH("%eax", "Push the pointer to byte 16 of the array record");
    SUB(16, "(%esp)", "Move it back to the start of the record");
    CALL("realloc", "Make the array record bigger");
    RELEASE_STACK(8, "Pop realloc's arguments");
    POP("%ecx", "%ecx = l-value of the array variable");
    ADD(16, "%eax", "Move eax forward to byte 16 of the record");
    MOV("%eax", "(%ecx)", "Reset the array variable to new record");

    // Fill the new array elements with their default value:
    // NOTE: This can be made more efficient later.
    PUSH("%eax", "Save reg for calls to cgx_push_default");
    PUSH("%ebx", "Save reg for calls to cgx_push_default");
    MOV("-4(%eax)", "%ecx", "%ecx = number of elements already with value");
    MOV("%ecx", "%edx", "%edx = ...");
    SHL(2, "%edx", "...number of bytes already with value");
    ADD("%eax", "%edx", "%edx = address of first uninitialized byte");
    LABEL(label_loop_top);
    PUSH("%ecx", "Save reg for call to cgx_push_default");
    PUSH("%edx", "Save reg for call to cgx_push_default");
    cgx_push_default(p->attribute<const tree*>("Type"));
    MOV("4(%esp)", "%edx", "Restore reg after call to cgx_push_default");
    MOV("8(%esp)", "%ecx", "Restore reg after call to cgx_push_default");
    POP("(%edx)", "Initialize the next array element");
    RELEASE_STACK(8, "Pop those two saved registers");
    ADD(4, "%edx", "%edx = address of next uninitialized byte");
    INC("%ecx", "%ecx = number of elements already with value");
    CMP("%ebx", "%ecx", "Have we initialized all?");
    JUMP("jle", label_loop_top, "If not, jump back to loop top"); 
    POP("%ebx", "Restore reg after calls to cgx_push_default");
    POP("%eax", "Restore reg after calls to cgx_push_default");

    // Reset the header data that tells how many elements are in the array:
    MOV("%ecx", "-4(%eax)", "Reset number of array elements");
    SHL(2, "%ecx", "%ecx = 4*ecx + ...");
    ADD(16, "%ecx", "...16 (total bytes of the array record)");
    MOV("%ecx", "-16(%eax)", "Reset size of array record");

    // eax points to byte 16 of the array record, and ebx is the index.
    // Compute and push the address of the element that we're after:
    LABEL(label);
    SHL(2, "%ebx", "%ebx = 4*index (offset of element)");
    ADD("%ebx", "%eax", "%eax = address of element");
    PUSH("%eax", "Push the address of array[i]");
}

void cgx_push_lval_expr__MINUSMINUS_expr(const tree* p)
{
    check(
	p->attribute<rhs>("RHS") == __MINUSMINUS_expr,
	"cgx_push_lval_expr__MINUSMINUS_expr"
	);

    // To do for HW 5.
}

void cgx_push_lval_expr__PLUSPLUS_expr(const tree* p)
{
    check(
	p->attribute<rhs>("RHS") == __PLUSPLUS_expr,
	"cgx_push_lval_expr__PLUSPLUS_expr"
	);

    // To do for HW 5.
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void cgx_push_rval_expr(const tree* p)
// Written by Michael Main (Feb 3, 2011)
// The pointer p must point to an <expr> node.  This function generates code
// that evaluates the expression and leaves a copy of the r-value of that
// expression on top of the stack.  If the expression is an array or string,
// then the copy is a deep copy.  This is always a 4-byte value:
// --An |int| is a 4-byte integer.
// --An |float| is a 4-byte float.
// --A |bool| is 0x00000000 for false and 0x00000001 for true.
// --A pointerto <type> is a 4-byte address that points
//   to a value of the required type.
// --An arrayof <type> is a 4-byte address of an array record that has these
//   components:
//   == (at address-16) the total number of bytes for this record.
//      This will always equal 16 + 4 times the current number of elements
//      in the array (allowing for the 16 bytes of information that precede
//      the [0] element of the array).
//   == (at address-12) a number telling something about the type of this
//      array's elements:
//      1: an array of strings, or an array of arrays
//      0: any other kind of array
//   == (at address-8) reserved for future use.
//   == (at address-4) the current number of elements in the array..
//   == (at address+4*i) the 4-byte value of the element at index [i].
// --A |string| is a 4-byte address of a record that has these
//   components:
//   == (at address-16) the total number of bytes for this record.
//      This will always equal 17 + the maximum number of characters that
//      the string can hold without allocating more memory (allowing for
//      the 16 bytes of information that precede the first data character
//      and the one byte for the null termination).
//   == (at address-12) the number -1 to indicate that this is a string record 
//   == (at address-8) the maximum number of characters that this string can
//      grow to without allocating more memory.
//   == (at address-4) the current number of characters in the string,
//      not counting the null terminator.
//   == (at address...) the current characters of the string itself
//      followed by a null terminator (a single zero byte).
{
    check(p->attribute<lhs>("LHS") == expr__, "cgx_push_rval_expr");

    switch(p->attribute<rhs>("RHS"))
    {
    case __INTEGERVALUE:
        cgx_push_rval_expr__INTEGERVALUE(p);
	break;
    case __FLOATVALUE:
        cgx_push_rval_expr__FLOATVALUE(p);
	break;
    case __STRINGVALUE:
        cgx_push_rval_expr__STRINGVALUE(p);
	break;
    case __TRUE:
	PUSH(1, "Push r-value of true constant");
	break;
    case __FALSE:
    case __NULLPTR:
	PUSH(0, "Push r-value of false or nullptr");
        break;
    case __IDENTIFIER:
        cgx_push_rval_expr__IDENTIFIER(p);
	break;
    case __IDENTIFIER_LPAREN_exprseq_RPAREN:
	cgx_push_rval_expr__IDENTIFIER_LPAREN_exprseq_RPAREN(p);
	break;
    case __LPAREN_ARRAY_OF_typeexpr_IS_exprseq_RPAREN:
	cgx_push_rval_expr__LPAREN_ARRAY_OF_typeexpr_IS_exprseq_RPAREN(p);
	break;
    case __LPAREN_NEW_typeexpr_IS_expr_RPAREN:
	cgx_push_rval_expr__LPAREN_NEW_typeexpr_IS_expr_RPAREN(p);
	break;
    case __PLUS_expr:
    case __LPAREN_expr_RPAREN:
        cgx_push_rval_expr(p->child(1));
	break;
    case __expr_OR_expr:
        cgx_push_rval_expr__expr_OR_expr(p);
	break;
    case __expr_AND_expr:
        cgx_push_rval_expr__expr_AND_expr(p);
	break;
    case __expr_LT_expr:
    case __expr_GT_expr:
    case __expr_LE_expr:
    case __expr_GE_expr:
    case __expr_EQEQ_expr:
    case __expr_NE_expr:
        cgx_push_rval_expr__expr_COMPARE_expr(p);
	break;
    case __expr_PLUS_expr:
        cgx_push_rval_expr__expr_PLUS_expr(p);
	break;
    case __expr_MINUS_expr:
        cgx_push_rval_expr__expr_MINUS_expr(p);
	break;
    case __expr_STAR_expr:
        cgx_push_rval_expr__expr_STAR_expr(p);
	break;
    case __expr_SLASH_expr:
        cgx_push_rval_expr__expr_SLASH_expr(p);
	break;
    case __expr_PERCENT_expr:
        cgx_push_rval_expr__expr_PERCENT_expr(p);
	break;
    case __expr_HAT_expr:
        cgx_push_rval_expr__expr_HAT_expr(p);
	break;
    case __MINUS_expr:
	cgx_push_rval_expr__MINUS_expr(p);
       	break;
    case __NOT_expr:
        cgx_push_rval_expr(p->child(1));
	NOT_TOP;
	break;
    case __STAR_expr:
	cgx_push_rval_expr__STAR_expr(p);
	break;
    case __AT_expr:
        cgx_push_lval_expr(p->child(1));
	break;
    case __FLOATCAST_expr:
	cgx_push_rval_expr__FLOATCAST_expr(p);
	break;
    case __ROUND_expr:
	cgx_push_rval_expr__ROUND_expr(p);
	break;
    case __expr_LSQUARE_expr_RSQUARE:
	cgx_push_rval_expr__expr_LSQUARE_expr_RSQUARE(p);
	break;
    case __expr_MINUSMINUS:
	cgx_push_rval_expr__expr_MINUSMINUS(p);
	break;
    case __expr_PLUSPLUS:
	cgx_push_rval_expr__expr_PLUSPLUS(p);
	break;
    case __MINUSMINUS_expr:
	cgx_push_rval_expr__MINUSMINUS_expr(p);
	break;
    case __PLUSPLUS_expr:
	cgx_push_rval_expr__PLUSPLUS_expr(p);
	break;
	break;
    default:
	check(false, "cgx_push_rval_expr");
	break;
    }
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// For each of the cgx_push_rval_expr__... functions, the pointer p must point
// to an <expr> node with a certain rhs. All of these functions generate code
// to leave the rvalue of the expression on top of the stack. See the
// documentation of cgx_push_rval_expr() for the format of this rvalue
// (which is always 4 bytes).
void cgx_push_rval_expr__INTEGERVALUE(const tree* p)
{
    check(p->attribute<rhs>("RHS") == __INTEGERVALUE, "cgx_push_rval_expr__INTEGERVALUE");

    // To do for HW 5.
}

void cgx_push_rval_expr__FLOATVALUE(const tree* p)
// Written by Michael Main (Feb 3, 2011)
{   
    check(p->attribute<rhs>("RHS") == __FLOATVALUE, "cgx_push_rval_expr__FLOATVALUE");

    int converted;
    float* pf = reinterpret_cast<float *>(&converted);
    
    *pf = atof(p->child(0)->label( ).c_str( ));
    PUSH(converted, p->child(0)->label() + " converted to int");
}

void cgx_push_rval_expr__STRINGVALUE(const tree* p)
// Written by Michael Main (Feb 3, 2011)
{
    check(p->attribute<rhs>("RHS") == __STRINGVALUE, "cgx_push_rval_expr__STRINGVALUE");
    const tree* type = p->attribute<const tree*>("Type");
    cgx_push_shallow_rval_expr__STRINGVALUE(p); // Shallow string
    cgx_make_deep_copy(type);
}

void cgx_push_rval_expr__IDENTIFIER(const tree* p)
// Written by Michael Main (Feb 3, 2011)
{
    check(p->attribute<rhs>("RHS") == __IDENTIFIER, "cgx_push_rval_expr__IDENTIFIER");
    const tree* type = p->attribute<const tree*>("Type");
    cgx_push_shallow_rval_expr__IDENTIFIER(p);
    if (is_using_implicit_memory(type))
	cgx_make_deep_copy(type);
}

void cgx_push_rval_expr__IDENTIFIER_LPAREN_exprseq_RPAREN(const tree* p)
// Written by Michael Main (Feb 3, 2011)
{
    check(
	p->attribute<rhs>("RHS") ==__IDENTIFIER_LPAREN_exprseq_RPAREN,
	"cgx_push_rval_expr__IDENTIFIER_LPAREN_exprseq_RPAREN"
	);
    
    // Call the function, which leaves its return value on top of the stack.
    // If this is a string or array, it will always be a deep copy.
    cgx_call(p);
}

void cgx_push_rval_expr__LPAREN_ARRAY_OF_typeexpr_IS_exprseq_RPAREN(const tree* p)
// Written by Michael Main (Feb 3, 2011)
{
    check(
	p->attribute<rhs>("RHS") == __LPAREN_ARRAY_OF_typeexpr_IS_exprseq_RPAREN,
	"cgx_push_rval_expr__LPAREN_ARRAY_OF_typeexpr_IS_exprseq_RPAREN"
	);
    const tree* type = p->attribute<const tree*>("Type");
    cgx_push_shallow_rval_expr__LPAREN_ARRAY_OF_typeexpr_IS_exprseq_RPAREN(p);
    cgx_make_deep_copy(type);
}

void cgx_push_rval_expr__LPAREN_NEW_typeexpr_IS_expr_RPAREN(const tree* p)
// Written by Michael Main (Feb 3, 2011)
// The parameter, p, can be a pointer to any ALLOCATE expression, with or
// without an initialization part.  If there is no initialization part,
// then the newly allocated variable is initialized with a default value.
// In either case, a pointer to the newly allocated memory is left on
// top of the stack.
{
    check(
	p->attribute<rhs>("RHS") == __LPAREN_NEW_typeexpr_IS_expr_RPAREN,
	"cgx_push_rval_expr__LPAREN_NEW_typeexpr_IS_expr_RPAREN"
	);
    const tree* need_type = p->child(2);
    const tree* have_type = p->child(4)->attribute<const tree*>("Type");
    
    // Push a value to store in the newly allocated memory:
    cgx_push_rval_expr(p->child(4));
    cgx_coerce_stack_top_to_float_if_needed(need_type, have_type);

    // Allocate the new memory and pop the value into it:
    PUSH(4, "Push malloc's argument");
    CALL("malloc", "Allocate 4 bytes of heap memory");
    RELEASE_STACK(4, "Pop malloc's arguments");
    POP("(%eax)", "Pop the initial value to newly allocated memory.");

    // Push the rvalue of the allocate expression, which is a pointer to
    // the newly allocated memory:
    PUSH("%eax", "Push the rvalue of the allocate expression");
}

void cgx_push_rval_expr__expr_OR_expr(const tree* p)
// Written by Michael Main (Feb 3, 2011)
{
    check(p->attribute<rhs>("RHS") == __expr_OR_expr, "cgx_push_rval_expr__expr_OR_expr");
    int label_number = unique_number( );
    
    cgx_push_rval_expr(p->child(0));    // The left operand
    CMP(1, "(%esp)", "Check left side of or");
    JUMP("je", label_number, "Skip evaluation of right side");
    RELEASE_STACK(4, "Discard left side of or");
    cgx_push_rval_expr(p->child(2));    // The right operand
    LABEL(label_number);
}

void cgx_push_rval_expr__expr_AND_expr(const tree* p)
{
    check(p->attribute<rhs>("RHS") == __expr_AND_expr, "cgx_push_rval_expr__expr_AND_expr");

    // To do for HW 5.
}

void cgx_push_rval_expr__expr_COMPARE_expr(const tree* p)
// Written by Michael Main (Feb 3, 2011)
// The pointer p must be a pointer to a node which is one of the six binary
// comparison expressions.  
// NOTE: Future plan to implement this for other types, too.
{
    check(
	p->attribute<rhs>("RHS") == __expr_LT_expr
	||
	p->attribute<rhs>("RHS") == __expr_GT_expr
	||
	p->attribute<rhs>("RHS") == __expr_LE_expr
	||
	p->attribute<rhs>("RHS") == __expr_GE_expr
	||
	p->attribute<rhs>("RHS") == __expr_EQEQ_expr
	||
	p->attribute<rhs>("RHS") == __expr_NE_expr,
	"cgx_push_rval_expr__expr_COMPARE_expr"
	);
    
    int label_number = unique_number( );
    
    PUSH(0, "Push false");
    cgx_jump_for_false_compare(p, label_number);
    NOT_TOP;
    LABEL(label_number);
}

void cgx_push_rval_expr__expr_HAT_expr(const tree* p)
// Written by Michael Main (Feb 3, 2011)
{
    check(p->attribute<rhs>("RHS") == __expr_HAT_expr, "cgx_push_rval_expr__expr_HAT_expr");
    const tree* type1 = p->child(0)->attribute<const tree*>("Type");
    const tree* type2 = p->child(2)->attribute<const tree*>("Type");

    if (is_compat(INTEGER_TYPE, type1) && is_compat(INTEGER_TYPE, type2))
    {
	cgx_push_rval_expr(p->child(2));    // The exponent
	cgx_push_rval_expr(p->child(0));    // The base
	POP("%eax", "%eax = base");
	POP("%ebx", "%ebx = exponent");
	CALL("lib.intpow", "Compute base^exponent");
	PUSH("%eax", "Push lib.intpow's result");
    }
    else
    {   // Float exponentiation:
	cgx_push_rval_expr(p->child(2));    // The exponent
	if (is_compat(INTEGER_TYPE, type2))
	    FILD("(%esp)", "Load a 4-byte float from top of stack");
	else
	    FLD("(%esp)", "Load a 4-byte float from top of stack");
	ALLOCATE_STACK(4, "4 more bytes, so room for 8-byte float");
	FSTP8("(%esp)", "Store the 8-byte float on top of the stack");

	cgx_push_rval_expr(p->child(0));    // The base
	if (is_compat(INTEGER_TYPE, type1))
	    FILD("(%esp)", "Load a 4-byte float from top of stack");
	else
	    FLD("(%esp)", "Load a 4-byte float from top of stack");
	ALLOCATE_STACK(4, "4 more bytes, so room for 8-byte float");
	FSTP8("(%esp)", "Store the 8-byte float on top of the stack");

	CALL("pow", "Call pow");
	RELEASE_STACK(12, "Remove 12 of 16 bytes of pow's arguments");
	FSTP("(%esp)", "And replace the other 4 with pow's answer");
    }
}

void cgx_push_rval_expr__expr_MINUS_expr(const tree* p)
// Written by Michael Main (Feb 3, 2011)
{
    check(p->attribute<rhs>("RHS") == __expr_MINUS_expr, "cgx_push_rval_expr__expr_MINUS_expr");
    const tree* type1 = p->child(0)->attribute<const tree*>("Type");
    const tree* type2 = p->child(2)->attribute<const tree*>("Type");

    if (is_compat(INTEGER_TYPE, type1) && is_compat(INTEGER_TYPE, type2))
    {
	cgx_push_rval_expr(p->child(0));    // The left operand
	cgx_push_rval_expr(p->child(2));    // The right operand
	POP("%eax", "The right operand of a subtraction");
	SUB("%eax", "(%esp)", "Do the subtraction");
    }
    else
    {   // Float subtraction:
	cgx_flop(p, "fsub", "fisub", "fisubr", false);
    }
}

void cgx_push_rval_expr__expr_PERCENT_expr(const tree* p)
// Written by Michael Main (Feb 3, 2011)
{
    check(p->attribute<rhs>("RHS") == __expr_PERCENT_expr, "cgx_push_rval_expr__expr_PERCENT_expr");

    cgx_push_rval_expr(p->child(0));    // The left operand
    cgx_push_rval_expr(p->child(2));    // The right operand
    POP("%ebx", "%ebx = denominator for division");
    POP("%eax", "%eax = numerator for division");
    CDQ("sign extend eax into edx:eax");
    IDIV("%ebx", "%eax = eax/ebx with remainder to edx");
    PUSH("%edx", "Push the remainder");
}

void cgx_push_rval_expr__expr_PLUS_expr(const tree* p)
{
    check(p->attribute<rhs>("RHS") == __expr_PLUS_expr, "cgx_push_rval_expr__expr_PLUS_expr");

    // To do for HW 5.
}

void cgx_push_rval_expr__expr_SLASH_expr(const tree* p)
// Written by Michael Main (Feb 3, 2011)
{
    check(p->attribute<rhs>("RHS") == __expr_SLASH_expr, "cgx_push_rval_expr__expr_SLASH_expr");
    const tree* type1 = p->child(0)->attribute<const tree*>("Type");
    const tree* type2 = p->child(2)->attribute<const tree*>("Type");

    if (is_compat(INTEGER_TYPE, type1) && is_compat(INTEGER_TYPE, type2))
    {
	cgx_push_rval_expr(p->child(0));    // The left operand
	cgx_push_rval_expr(p->child(2));    // The right operand
	POP("%ebx", "%ebx = denominator for division");
	POP("%eax", "%eax = numerator for division");
	CDQ("sign extend eax into edx:eax");
	IDIV("%ebx", "%eax = eax/ebx");
	PUSH("%eax", "Push the quotient");
    }
    else
    {   // Float divide:
	cgx_flop(p, "fdiv", "fidiv", "fidivr", false);
    }
}

void cgx_push_rval_expr__expr_STAR_expr(const tree* p)
// Written by Michael Main (Feb 3, 2011)
{
    check(p->attribute<rhs>("RHS") == __expr_STAR_expr, "cgx_push_rval_expr__expr_STAR_expr");
    const tree* type1 = p->child(0)->attribute<const tree*>("Type");
    const tree* type2 = p->child(2)->attribute<const tree*>("Type");

    if (is_compat(INTEGER_TYPE, type1) && is_compat(INTEGER_TYPE, type2))
    {
	cgx_push_rval_expr(p->child(0));    // The left operand
	cgx_push_rval_expr(p->child(2));    // The right operand
	POP("%eax", "The right operand of a multiplication");
	IMUL("(%esp)", "%eax *= left operand");
	RELEASE_STACK(4, "Pop the left operand");
	PUSH("%eax", "Push the answer, ignoring any overflow");
    }
    else
    {   // Float multiply:
	cgx_flop(p, "fmul", "fimul", "fimul", false);
    }
}

void cgx_push_rval_expr__FLOATCAST_expr(const tree* p)
{
    check(
	p->attribute<rhs>("RHS") == __FLOATCAST_expr,
	"cgx_push_rval_expr__FLOATCAST_expr"
	);

    // To do for HW 5.
}

void cgx_push_rval_expr__MINUS_expr(const tree* p)
// Written by Michael Main (Feb 3, 2011)
{
    check(
	p->attribute<rhs>("RHS") == __MINUS_expr,
	"cgx_push_rval_expr__MINUS_expr"
	);
    
    cgx_push_rval_expr(p->child(1));
    if (is_compat(INTEGER_TYPE, p->child(1)->attribute<const tree*>("Type")))
	NEG_TOP;
    else
    {
	FLD("(%esp)", "Load float to float stack");
	FCHS("Change its sign");
	FSTP("(%esp)", "And store back on stack");
    }
}

void cgx_push_rval_expr__expr_MINUSMINUS(const tree* p)
{
    check(
	p->attribute<rhs>("RHS") == __expr_MINUSMINUS,
	"cgx_push_rval_expr__expr_MINUSMINUS"
	);
	

	

    // To do for HW 5.
}

void cgx_push_rval_expr__expr_PLUSPLUS(const tree* p)
{
    check(
	p->attribute<rhs>("RHS") == __expr_PLUSPLUS,
	"cgx_push_rval_expr__expr_PLUSPLUS"
	);

    // To do for HW 5.
}

void cgx_push_rval_expr__ROUND_expr(const tree* p)
// Written by Michael Main (Feb 3, 2011)
{
    check(
	p->attribute<rhs>("RHS") == __ROUND_expr,
	"cgx_push_rval_expr__ROUND_expr"
	);

    // To do for HW 5.
}

void cgx_push_rval_expr__STAR_expr(const tree* p)
// Written by Michael Main (Feb 3, 2011)
{
    check(
	p->attribute<rhs>("RHS") == __STAR_expr,
	"cgx_push_rval_expr__STAR_expr"
	);

    const tree* type = p->attribute<const tree*>("Type");
    cgx_push_shallow_rval_expr__STAR_expr(p);
    if (is_using_implicit_memory(type))
	cgx_make_deep_copy(type);
}

void cgx_push_rval_expr__expr_LSQUARE_expr_RSQUARE(const tree* p)
// Written by Michael Main (Feb 3, 2011)
// NOTE: There is no range checking, but that might be something
// for a future class to implement.
{
    check(
	p->attribute<rhs>("RHS") == __expr_LSQUARE_expr_RSQUARE,
	"cgx_push_rval_expr__expr_LSQUARE_expr_RSQUARE"
	);
    const tree* parr = p->child(0);
    const tree* pindex = p->child(2);
    bool is_element_using_implicit_memory =
        is_using_implicit_memory(p->attribute<const tree*>("Type"));
    bool is_shallow_copy_of_array_possible =
        parr->attribute<bool>("Addressable");
    
    // Set ebx to point to element [0], and set ecx to the index:
    if (is_shallow_copy_of_array_possible)
	cgx_push_shallow_rval_expr(parr); // Shallow array
    else
	cgx_push_rval_expr(parr);  // Push deep copy of the array
    cgx_push_rval_expr(pindex);
    POP("%ecx", "%ecx = index");
    POP("%ebx", "%ebx = ptr to [0] array elemet");

    // Compute the element's address as ebx+4*ecx, and push the item
    // from this location of the array onto the stack:
    PUSH("(%ebx,%ecx,4)", "Push shallow copy of array element");

    // If the item we just pushed uses implicit memory, then we must
    // make a deep copy of it.  Note that lib.copyrec preserves ebx.
    if (is_element_using_implicit_memory)
    {   
	MOV("%esp", "%eax", "%eax = lib.copyrec's argument");
	CALL("lib.copyrec", "Deep copy of an array element");
    }

    // If we needed to make a deep copy of the array, then we must
    // free that deep copy now.
    if (!is_shallow_copy_of_array_possible)
    {
	MOV("%ebx", "%eax", "%eax = pointer to byte 16 of array record");
	CALL("lib.freerec", "Free the deep copy of that array");
    }
}

void cgx_push_rval_expr__MINUSMINUS_expr(const tree* p)
{
    check(
	p->attribute<rhs>("RHS") == __MINUSMINUS_expr,
	"cgx_push_rval_expr__MINUSMINUS_expr"
	);

    // To do for HW 5.
}

void cgx_push_rval_expr__PLUSPLUS_expr(const tree* p)
{
    check(
	p->attribute<rhs>("RHS") == __PLUSPLUS_expr,
	"cgx_push_rval_expr__PLUSPLUS_expr"
	);

    // To do for HW 5.
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void cgx_push_shallow_rval_expr(const tree* p)
// Written by Michael Main (Feb 3, 2011)
// The pointer p must point to an <expr> node for which
// p->attribute<bool>("Addressable") s true.
// This function generates code that evaluates the
// expression and leaves a shallow copy of the r-value of that
// expression on top of the stack (one that does not duplicate any
// dynamic memory that the value uses).
//
// A shallow copy does not make a new copy of the array or string record.
// Instead, it just pushes the contents of the current array or string,
// which is a pointer to a previously existing array or string record.
{
    check(p->attribute<lhs>("LHS") == expr__, "cgx_push_shallow_rval_expr");
    assert(p->attribute<bool>("Addressable"));
	   
    switch(p->attribute<rhs>("RHS"))
    {
    case __IDENTIFIER:
        cgx_push_shallow_rval_expr__IDENTIFIER(p);
	break;
    case __STRINGVALUE:
        cgx_push_shallow_rval_expr__STRINGVALUE(p);
	break;
    case __LPAREN_expr_RPAREN:
        cgx_push_shallow_rval_expr(p->child(1));
	break;
    case __expr_LSQUARE_expr_RSQUARE:
	cgx_push_shallow_rval_expr__expr_LSQUARE_expr_RSQUARE(p);
	break;
    case __STAR_expr:
	cgx_push_shallow_rval_expr__STAR_expr(p);
	break;
    case __LPAREN_ARRAY_OF_typeexpr_IS_exprseq_RPAREN:
	cgx_push_shallow_rval_expr__LPAREN_ARRAY_OF_typeexpr_IS_exprseq_RPAREN(p);
	break;
    default:
	// For other kinds of expressions, a shallow rvalue is not possible.
	check(false, "cgx_push_shallow_rval_expr");
	break;
    }
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// For each of the cgx_push_shallow_rval_expr__... functions, the pointer p
// must point to an <expr> node with a certain rhs.  The value of
// p->attribute<bool>("Addressable") must be true.  All of these functions
// generate code to leave a shallow copy of the
// rvalue on top of the stack. See the documentation on data types (at the top
// of this file) for the format of this rvalue (which is always 4 bytes).
void cgx_push_shallow_rval_expr__STRINGVALUE(const tree* p)
// Written by Michael Main (Feb 3, 2011)
{
    check(p->attribute<rhs>("RHS") == __STRINGVALUE, "cgx_push_shallow_rval_expr__STRINGVALUE");
    PUSH("$" + cgx_define_string_constant(p->child(0)), "Pointer to a constant string");
}

void cgx_push_shallow_rval_expr__IDENTIFIER(const tree* p)
// Written by Michael Main (Feb 3, 2011)
{
    check(p->attribute<rhs>("RHS") == __IDENTIFIER, "cgx_push_shallow_rval_expr__IDENTIFIER");
    char op[MAX_OPERAND];
    const tree* leaf = p->child(0);
    int identifier_depth = leaf->attribute<int>("Depth");
    int offset = leaf->attribute<int>("Offset");
    int distance = current_depth - identifier_depth;
    
    // Push the r-value of the variable
    if (identifier_depth == 0)
    {   // Global variable
        sprintf(op, "(compiler.globals.base+%d)", offset);
        PUSH(op, "Static variable");
    }
    else if (distance == 0)
    {   // A local variable or parameter of the current frame
	// Part of HW 7. Without this part of the code generator,
	// only global variables will work in a CU program.
    }
    else
    {   // A variable or parameter in a distant frame
	// Part of HW 7. Without this part of the code generator,
	// only global variables will work in a CU program.
    }
}

void cgx_push_shallow_rval_expr__LPAREN_ARRAY_OF_typeexpr_IS_exprseq_RPAREN(const tree* p)
// Written by Michael Main (Feb 3, 2011)
{
    check(p->attribute<rhs>("RHS") == __LPAREN_ARRAY_OF_typeexpr_IS_exprseq_RPAREN,
	"cgx_push_shallow_rval_expr__LPAREN_ARRAY_OF_typeexpr_IS_exprseq_RPAREN"
	);

    // Create a new array record in static memory, and push a pointer to this
    // array record onto the stack.  We do not make a new deep copy of the
    // array record.
    PUSH("$" + cgx_define_array_constant(p), "Pointer to byte 16 of array record");
}

void cgx_push_shallow_rval_expr__STAR_expr(const tree* p)
// Written by Michael Main (Feb 3, 2011)
{
    check(
	p->attribute<rhs>("RHS") == __STAR_expr,
	"cgx_push_shallow_rval_expr__STAR_expr"
	);

    // Push a copy of the pointer, and dereferece the pointer, but do not
    // make a deep copy of the result.
    cgx_push_rval_expr(p->child(1));    // Push a copy of the pointer
    POP("%eax", "%eax = a pointer value");
    PUSH("(%eax)", "Push dereferenced pointer value");
}

void cgx_push_shallow_rval_expr__expr_LSQUARE_expr_RSQUARE(const tree* p)
// Written by Michael Main (Feb 3, 2011)
{
    check(
	p->attribute<rhs>("RHS") ==
	__expr_LSQUARE_expr_RSQUARE,
	"cgx_push_shallow_rval_expr__expr_LSQUARE_expr_RSQUARE"
	);
    const tree* parr = p->child(0);
    const tree* pindex = p->child(2);
    
    // Push a shallow copy of the array.  This must always be possible
    // because the decorator requires that the whole array has an address
    // in order for its individual elements to have an address.
    cgx_push_shallow_rval_expr(parr);

    // Compute the element's address as ebx+4*ecx, and push the item
    // from this location of the array onto the stack:
    cgx_push_rval_expr(pindex);
    POP("%ecx", "%ecx = index");
    POP("%ebx", "%ebx = ptr to [0] array elemet");
    PUSH("(%ebx,%ecx,4)", "Push shallow copy of array element");
}

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void cgx_push_static_link(int depth)
// Written by Michael Main (Feb 3, 2011)
// This function generates code that computes and pushes the static
// link onto the stack.  This occurs when the function's frame is first
// being created.  The depth argument is the depth of the blockstmt that
// is opening the function's frame.  For the blockstmt that defines
// a global function (depth == 0), the static link for its function's frame
// is NULL (since there is no larger enclosing static scope).
// For any other blockstmt, we assume that:
// 1. The ebp is still pointing to the base of the old function's frame.
// 2. The static depth of the new function's frame has just been pushed
//    onto the stack.
// 3. The static depth of the previous function's frame is at 8(%epb)
//    in the old function's frame.
{
    if (depth == 0)
    {   // Top-level functions have a NULL static link:
	PUSH(0, "NULL static link");
    }
    else
    {
	// This is part of HW 7. Note: It is called from cgx_call; without
	// this code, correct static links will not be present in the frame
	// of a nested function.
    }
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void cgx_read(const tree* type)
// Written by Michael Main (Feb 3, 2011)
// This function generates code to read a value of the specified type.
// The value is read into an l-value that is already on the stack prior
// to executing the code that this function generates.  After the code
// finishes, that l-value address has been popped from the stack.
{
    check(type->attribute<lhs>("LHS") == typeexpr__, "cgx_read");
    
    if (is_compat(INTEGER_TYPE, type))
    {   // Read an integer:
	// To do for HW 5.
    }
    else if (is_compat(STRING_TYPE, type))
    {   // Read a string:
	CALL("lib.readstr", "Read the string");
	RELEASE_STACK(4, "Pop lib.readstr's argument");
    }
    else if (is_compat(FLOAT_TYPE, type))
    {   // Read a float.
	// I am surprised the %f for scanf means a 4-byte float, but
	// for printf it was an 8-byte float.
	PUSH("$compiler.floatformat", "Push format parameter for float");
	CALL("scanf", "Call scanf");
	RELEASE_STACK(8, "Pop scanf's arguments");
    }
    else
    {   // Read a boolean:
	PUSH("$compiler.booleaninput", "Push lib.readstr's argument");
	CALL("lib.readstr", "Read a string");
	PUSH("$compiler.false", "Push strcmp's s1 arguent");
	PUSH("(compiler.booleaninput)", "Push strcmp's s2 argument");
	CALL("strcmp", "Was input string FALSE?");
	RELEASE_STACK(12, "Pop down to the boolean's l-value");
	POP("%ecx", "%ecx = l-value of the boolean being read");
	MOV("%eax", "(%ecx)", "Set the boolean value");
    }
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void cgx_set_carry_flag_from_floats(const tree* p1, const tree* p2)
// Written by Michael Main (Feb 3, 2011)
{
    const tree* type1 = p1->attribute<const tree*>("Type");
    const tree* type2 = p2->attribute<const tree*>("Type");

    cgx_push_rval_expr(p1);
    cgx_coerce_stack_top_to_float_if_needed(FLOAT_TYPE, type1);
    cgx_push_rval_expr(p2);
    cgx_coerce_stack_top_to_float_if_needed(FLOAT_TYPE, type2);
    FLD("(%esp)", "Load right op of comparison");
    FLD("4(%esp)", "Load left op of comparison");
    RELEASE_STACK(8, "Release the ops from the stack");
    cout << "fcompp" << endl;    // Floating point compare
    cout << "fnstsw %ax" << endl; // Move fp status word to ax register
    cout << "sahf" << endl;      // Move ah part of ax into flags
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void cgx_set_compare_flags(const tree* p)
// Written by Michael Main (Feb 3, 2011)
// The pointer p must be a pointer to a node which is one of the binary
// integer comparison expressions.  This function generates code that
// evaluates this comparison, setting the comparison flags for a subsequent
// jump.  For example, if the comparison is between 8 and 30, then a subsequent
// jl, jle or jne will be taken, but a jg, jge or je will not be taken.
// The stack is left unaltered.
{
    check(
	p->attribute<rhs>("RHS") == __expr_LT_expr
        ||
	p->attribute<rhs>("RHS") == __expr_GT_expr
	||
	p->attribute<rhs>("RHS") == __expr_LE_expr
	||
        p->attribute<rhs>("RHS") == __expr_GE_expr
	||
	p->attribute<rhs>("RHS") == __expr_EQEQ_expr
	||
	p->attribute<rhs>("RHS") == __expr_NE_expr,
	"cgx_set_compare_flags"
	);

    // Compare two integer values:
    cgx_push_rval_expr(p->child(0));
    cgx_push_rval_expr(p->child(2));
    POP("%eax", "Pop right operand into eax");
    POP("%ecx", "Pop left operand into ecx");
    CMP("%eax", "%ecx", "Set flags as if computing ecx - eax");
}
//-----------------------------------------------------------------------------


//-------------------------------------------------------------------------
bool is_array(const tree* type)
// Written by Michael Main (Feb 3, 2011)
// Precondition: tt must be a type tree (i.e., a parse tree with lhs of
// typeexpr__).
// Return value is true if this data type is an array
// (but not for a pointer to one of those).
{
    return tt_minus(type, ARRAY) != NULL;
}
//-------------------------------------------------------------------------


//-------------------------------------------------------------------------
bool is_complex_array(const tree* type)
// Written by Michael Main (Feb 3, 2011)
// Precondition: tt must be a type tree (i.e., a parse tree with lhs of
// typeexpr__).
// Return value is true if this data type is an array
// with components that are strings or arrays of some sort
// (but not for a pointer to one of those).
{
    const tree* subtree = tt_minus(type, ARRAY);

    return
        subtree != NULL
        &&
        is_using_implicit_memory(subtree);
}
//-------------------------------------------------------------------------


//-------------------------------------------------------------------------
bool is_simple_array(const tree* type)
// Written by Michael Main (Feb 3, 2011)
// Precondition: tt must be a type tree (i.e., a parse tree with lhs of
// typeexpr__).
// Return value is true if this data type is an array
// with components that are neither strings nor arrays of some sort
// (but not for a or pointer to one of those).
{
    const tree* subtree = tt_minus(type, ARRAY);

    return
        subtree != NULL
        &&
        !is_using_implicit_memory(subtree);
}
//-------------------------------------------------------------------------


//-------------------------------------------------------------------------
bool is_string(const tree* type)
// Written by Michael Main (Feb 3, 2011)
// Precondition: tt must be a type tree (i.e., a parse tree with lhs of
// typeexpr__).
// Return value is true if this data type is a string.
// (but not for an array of or pointer to one of those).
{
    return
	type->attribute<rhs>("RHS") == __TYPENAME
        &&
        type->child(0)->label( ) == "|string|";
}
//-------------------------------------------------------------------------


//-------------------------------------------------------------------------
bool is_using_implicit_memory(const tree* type)
// Written by Michael Main (Feb 3, 2011)
// Precondition: tt must be a type tree (i.e., a parse tree with lhs of
// typeexpr__).
// Return value is true if this data type is an array or string.
// (but not for a pointer to one of those).
{
    return is_string(type) || is_array(type);
}
//-------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// These functions allow us to print assembly instructions in a standard
// format. They should not be called directly, but only through the various
// macros (such as ADD, SUB...) at the top of this file.
string jump_label(int j)
{
    char label[MAX_OPERAND];
    sprintf(label, "jump.%d", j);
    return label;
}

string jump_label(string j)
{
    return "jump." + j;
}

void print_instruction(string comment, string inst, string op1, string op2)
{
    cout << "  " << inst << setw(6-inst.length()) << " " << op1;
    if (op2.length( ) != 0)
    {
	cout << ", " << setw(TAB-10-op1.length()) << op2;
    }
    else
    {
	cout << setw(TAB-8-op1.length()) << "";
    }
    cout << " # " << comment << endl;
}

void print_instruction(string comment, string inst, int op1, string op2)
{
    char sop1[MAX_OPERAND];
    sprintf(sop1, "$%d", op1);
    print_instruction(comment, inst, sop1, op2);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
int unique_number( )
// Written by Michael Main (Feb 3, 2011)
// The first time this function is called, it returns 1. The next time, it
// returns 2. Then 3, then 4, and so on. These numbers are used as part
// of a label in any jump statement.
{
    static int answer = 0;
    return ++answer;
}
//-----------------------------------------------------------------------------
