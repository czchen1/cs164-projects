/* -*- mode: fundamental; c-file-style: "stroustrup"; indent-tabs-mode: nil -*- */

/* apyc: Bison Parser */

/* Authors: YOUR NAMES HERE */

%{
  #include <iostream>
  #include <string>
  #include "apyc.h"
  #include "ast.h"

  using namespace std;

  static AST_Ptr theTree;

  /* Utilities */

  /** Type used for locations (@1, @2, etc.). */
  #define YYLTYPE Location

  #define YYLLOC_DEFAULT(Current, Rhs, N)                                    \
      do  {                                                                  \
         if (N) {                                                            \
             Current = YYRHSLOC(Rhs, 1);                                     \
         } else {                                                            \
             Current = YYRHSLOC(Rhs, 0);                                     \
         }                                                                   \
      } while (0)                                                            \

  static void yyerror (const char* s);

  static int yylex ();

  static string MAX_INT = "1073741823";
%}

%locations
%debug

/* A token declaration such as
 *   %token INDENT
 * declares a new integer constant INDENT, which serves as a syntactic category
 * (suitable for output by the lexer) identifying a kind of token, which we
 * also often refer to as INDENT. A declaration such as
 *   %token IF "if"
 * defines a constant IF and makes it the syntactic category for the token
 * "if".  The built-in function yyexternal_token_name(...) maps between 
 * syntactic categories (like IF) and the token strings defined in these 
 * %://gyazo.com/796d3e89d65f24e2da751ecc245d6f4btoken declarations (like "if").  
 *
 */

/* Lexer tokens and nonterminal instances that are referred to by $$, $1,
 * $2, etc. have a semantic value, whose type is declared by the <...>
 * notations in %token and %type declarations.  The names in
 * the <...> brackets refer to members of the %union declaration (see below).
 * Symbols whose semantic value is never referenced do not need types.
 */ 

%token<str> ID STRING INT NONE TRUE FALSE AND OR IF ELSE ELIF BREAK CONTINUE RETURN WHILE IN FOR NATIVE OF TYPEASSIGN TYPEVAR
%token<str> CLASS DEF PASS ARROW

%token NEWLINE INDENT DEDENT
%token PRINT "print"

%left OR
%left AND
%left NOT
%left '<' '>' LEQ GEQ EQ_OP NEQ IS IS_NOT NOT_IN
%left '-' '+'
%left '*' '/' '%'
%right POW

%type<ast> print_stmt expression and_or_expression not_expression expression0 expression1 left_cmpr string_literal tuple_expression
%type<ast> return_stmt while_stmt for_stmt if_stmt elif_expr 
%type<ast> id special_id var_ref pair factor factor1 primary 
%type<ast> right_side assign_statement target
%type<ast> typed_identifier type type_id type_list
%type<ast> simple_stmt compound_stmt statement simple_stmt_list suite native_stmt
%type<ast> class_decl class_block def_decl def_block formals_list typed_id type_var
%type<ast> target_list target_list1

%type<astVect> module_stmt_list type_formals class_stmt_list def_stmt_list formals_list1
%type<astVect> expression_list expression_list1 pair_list pair_list1 target_list2 print_expr println_expr
%type<astVect> tuple_expression1
%type<astVect> simple_stmt_list1 statement_list 
%type<astVect> type_list0

/* The following declaration provides names for possible semantic types
 * passed by the compiler.  It actually translates to a C++ union type.
 * This is like a class, but its fields are mutually exclusive.  For
 * technical reasons, all these types must be "POD (Plain Old Data) types";
 * they can't have constructors or other fancy C++ features.  Therefore,
 * they are all numeric types or pointer types. */

%union {
    std::string* str;
    AST_Ptr ast;
    AST_Vect* astVect;
}

/* GLR parsing is useful for cases where the parser must make a reduction before
 * it has sufficient information to know which reduction to make.  For example,
 * consider a statement that begins
 *      a[i], b[j], c
 * (it's not useful, but is valid Python as it stands).  As a tuple, this 
 * consists of a list of expressions.  a[i] is a subscription, and corresponds
 * to a (subscript ...) AST.  But if we have
 *      a[i], b[j], c = L
 * then the a[i] should create a subscript_assign.  If you try to handle this
 * by having two rules for subscription, one for the first case and one 
 * for the second, the parser has to decide which to choose just after 
 * seeing a[i], but long before seeing "=", which is how one tells which
 * translation is appropriate.  A GLR parser will hold off on this decision
 * until is reaches the =. */

%glr-parser

/* If you use the GLR features, Bison will report conflicts.  Without GLR,
 * a conflict generally means an error.  With GLR, it could mean an error,
 * or it could indicate conflicts that the GLR algorithm will correctly resolve.
 * There's really no way to know without testing.  When you get conflicts
 * THAT YOU EXPECT you can record here these expectations by reporting how
 * many reduce/reduce (%expect-rr) and shift-reduce (%expect) you expect. 
 * Before increasing these counts, make sure you understand what causes your
 * conflicts and that GLR will resolve them.  Don't expect too many
 * and try to remove conflicts rather than tolerating them.
 * A conflict will occur when there is a situation where Bison cannot decide
 * on the basis of the input so far whether to perform a reduction.  The 
 * conflict is OK if it will become clear later on in the input what should
 * have been done. */
%expect-rr 24

%expect 1

%%

/* See the ast.h file for declarations of NODE and LEAF.  NODE is for
 * creating AST nodes that have 0 or more children and no string content,
 * while LEAF is for creating childless nodes whose principal semantic
 * content is a string or integer value. */

        /***** MODULE *****/

/* Root node. Every node is a child of this. Return type AST */
program : module_stmt_list          { theTree = NODE(MODULE_AST, $1); }
  | /* empty */                     { theTree = NODE(MODULE_AST); }
  ;

/* All statements that can be placed at the module level: classes, functions, compound/simple statements */
/* Return type AST_Vect */
module_stmt_list :
    class_decl                      { $$ = new AST_Vect(1, $1); }
  | def_decl                        { $$ = new AST_Vect(1, $1); }
  | statement                       { $$ = new AST_Vect(1, $1); }
  | module_stmt_list class_decl     { $$ = $1; $$->push_back($2); }
  | module_stmt_list def_decl       { $$ = $1; $$->push_back($2); }
  | module_stmt_list statement      { $$ = $1; $$->push_back($2); }
  ;

        /***** CLASSES *****/

/* Class statement - can be child to module only! Return type AST */
class_decl :
    CLASS id ':' class_block                              { AST_Ptr n = NODE(TYPE_FORMALS_LIST_AST); n->setLoc(@1);
                                                            $$ = NODE(CLASS_AST, $2, n, $4); $$->setLoc(@1); }
  | CLASS id OF '[' type_formals ']' ':' class_block      { AST_Ptr n = NODE(TYPE_FORMALS_LIST_AST, $5); n->setLoc(@4);
                                                            $$ = NODE(CLASS_AST, $2, n, $8); $$->setLoc(@1); }
  ;

/* corresponds to TYPE_FORMALS_LIST - return type AST_Vect */
type_formals :
    type_var                        { $$ = new AST_Vect(1, $1); }
  | type_formals ',' type_var       { $$ = $1; $$->push_back($3); }
  ;

/* class equivalent of suite. */
/* Return type AST */
class_block :
    simple_stmt_list NEWLINE                      { $$ = NODE(BLOCK_AST, $1); $$->setLoc(@1); }
  | NEWLINE INDENT class_stmt_list NEWLINE DEDENT { $$ = NODE(BLOCK_AST, $3); $$->setLoc(@1); }
  | NEWLINE INDENT class_stmt_list DEDENT         { $$ = NODE(BLOCK_AST, $3); $$->setLoc(@1); }
  ;

/* All statements that can be placed at the class level: everything but classes. */
/* Return type AST_Vect */
class_stmt_list :
    def_decl                                    { $$ = new AST_Vect(1, $1); }
  | statement                                   { $$ = new AST_Vect(1, $1); }
  | class_stmt_list def_decl                    { $$ = $1; $$->push_back($2); }
  | class_stmt_list statement                   { $$ = $1; $$->push_back($2); }
  ;

                /***** FUNCTIONS *****/

/* Def statement - can be child to module, class, or def. Return type AST */
def_decl :
    DEF id '(' ')' ':' def_block                        { AST_Ptr n = NODE(FORMALS_LIST_AST); n->setLoc(@3);
                                                          $$ = NODE(DEF_AST, $2, n, $6); $$->setLoc(@1); }
  | DEF id '(' formals_list ')' ':' def_block           { $$ = NODE(DEF_AST, $2, $4, $7); $$->setLoc(@1); }
  | DEF id '(' ')' TYPEASSIGN type ':' def_block        { AST_Ptr n = NODE(FORMALS_LIST_AST); n->setLoc(@3);
                                                          $$ = NODE(DEF_AST, $2, n, $6, $8); $$->setLoc(@1); }
  | DEF id '(' formals_list ')' TYPEASSIGN type ':' def_block       { $$ = NODE(DEF_AST, $2, $4, $7, $9); $$->setLoc(@1); }
  ;

/* function equivalent of suite. Return type AST */
def_block :
      simple_stmt_list NEWLINE                        { $$ = NODE (BLOCK_AST, $1); $$->setLoc(@1); }
    | NEWLINE INDENT def_stmt_list NEWLINE DEDENT     { $$ = NODE (BLOCK_AST, $3); $$->setLoc(@1); } /* TODO: fix DEDENT */
    | NEWLINE INDENT def_stmt_list DEDENT             { $$ = NODE (BLOCK_AST, $3); $$->setLoc(@1); }
    ;

/* All statements that can be placed at the function level: everything but classes. Return type AST_Vect */
def_stmt_list :
    statement                       { $$ = new AST_Vect(1, $1); }
  | def_decl                        { $$ = new AST_Vect(1, $1); }
  | def_stmt_list statement         { $$ = $1; $$->push_back($2); }
  | def_stmt_list def_decl          { $$ = $1; $$->push_back($2); }
  ;

                /*** STATEMENTS ***/

/* Non-empty list of stmt_lists = simple_stmt; simple_stmt ; ... */
statement_list :
      statement                           { $$ = new AST_Vect(1, $1); }
    | statement_list statement            { $$ = $1; $$->push_back($2); }
    ;

/* Each statement corresponds to a statement node. The node can represent
a semicolon separated list of simple statements OR a single compound stmt.*/
statement :
      simple_stmt_list NEWLINE { $$ = $1; }
    | compound_stmt
    ;

/* Node representing semicolon separated simple statements on one line.
If there are two or more statements, a parent statement node is created.
With one statement on the line, that node is simply returned. */
simple_stmt_list :
      simple_stmt_list1             { $$ = NODE (STMT_LIST_AST, $1); $$->setLoc(@1); }
    | simple_stmt_list1 ';'         { $$ = NODE (STMT_LIST_AST, $1); $$->setLoc(@1); }
    | simple_stmt
    | simple_stmt ';'        { $$ = $1; }


simple_stmt_list1:
      simple_stmt ';' simple_stmt         { $$ = new AST_Vect(1, $1); $$->push_back($3); } 
    | simple_stmt_list1 ';' simple_stmt   { $$ = $1; $1->push_back($3); }
    ;

/* Statements that can fit on one line. Returns a node - type is not stmt_list_ast. */
simple_stmt :
      tuple_expression
    | assign_statement
    | print_stmt
    | BREAK           { $$ = LEAF(BREAK_AST, "", @1); }
    | CONTINUE        { $$ = LEAF(CONTINUE_AST, "", @1); }
    | return_stmt
    | native_stmt
    | PASS            { $$ = NODE(STMT_LIST_AST); $$->setLoc(@1); }
    ;

/* Multiline statements - must be terminated by a newline/dedent 
Technically for/while/if stmts can be one line, but they cannot be chained
with other stmts in a semicolon separated list on the same line.*/
compound_stmt :
      while_stmt
    | for_stmt
    | if_stmt
    ;

while_stmt :
      WHILE expression ':' suite                { $$ = NODE (WHILE_AST, $2, $4); $$->setLoc(@1); }
    | WHILE expression ':' suite ELSE ':' suite { $$ = NODE (WHILE_AST, $2, $4, $7); $$->setLoc(@1); }
    ;

for_stmt : 
      FOR target_list IN tuple_expression ':' suite                   { $$ = NODE (FOR_AST, $2, $4, $6); $$->setLoc(@1); }
    | FOR target_list IN tuple_expression ':' suite  ELSE ':' suite   { $$ = NODE (FOR_AST, $2, $4, $6, $9); $$->setLoc(@1); }
    ;

if_stmt :
    IF expression ':' suite                         { $$ = NODE (IF_AST, $2, $4); $$->setLoc(@1); }
  | IF expression ':' suite elif_expr               { $$ = NODE (IF_AST, $2, $4, $5); $$->setLoc(@1); }
  | IF expression ':' suite ELSE ':' suite          { $$ = NODE (IF_AST, $2, $4, $7); $$->setLoc(@1); }
  ;

elif_expr :
    ELIF expression ':' suite                       { $$ = NODE (IF_AST, $2, $4); $$->setLoc(@1); }
  | ELIF expression ':' suite ELSE ':' suite        { $$ = NODE (IF_AST, $2, $4, $7); $$->setLoc(@1); }
  | ELIF expression ':' suite elif_expr             { $$ = NODE (IF_AST, $2, $4, $5); $$->setLoc(@1); }
;

suite :
      simple_stmt_list NEWLINE                        { $$ = $1; }
    | NEWLINE INDENT statement_list NEWLINE DEDENT    { $$ = NODE (STMT_LIST_AST, $3); $$->setLoc(@1); }
    | NEWLINE INDENT statement_list DEDENT            { $$ = NODE (STMT_LIST_AST, $3); $$->setLoc(@1); }
    ;

print_stmt : 
      "print" print_expr ','          { $$ = NODE (PRINT_AST, $2); $$->setLoc(@1); }
    | "print" println_expr            { $$ = NODE (PRINTLN_AST, $2); $$->setLoc(@1); }
    ;

println_expr :
     /*empty*/                        { $$ = new AST_Vect(); }
    | expression_list1
    ;

print_expr :
      expression_list1
    ;

return_stmt :
        RETURN tuple_expression       { $$ = NODE (RETURN_AST, $2); $$->setLoc(@1); }
      | RETURN /*empty*/              { $$ = NODE (RETURN_AST, LEAF(NONE_AST, "", @1)); $$->setLoc(@1); } 
      ;

native_stmt :
        NATIVE string_literal         { $$ = NODE (NATIVE_AST, $2); $$->setLoc(@1); }
      ;

               /*** ASSIGNMENT ***/
assign_statement :
      target_list '=' right_side      { $$ = NODE(ASSIGN_AST, $1, $3); $$->setLoc(@2); } 
    ;
 
/* Target_list is a vector of targets. Targets are id, attributeref, subscription,
slicing, and a list or tuple of targets.  */
target_list :
      target_list1
    | '[' target ']'                  { $$ = NODE(TARGET_LIST_AST, $2); $$->setLoc(@1); }
    | '[' target ',' ']'              { $$ = NODE(TARGET_LIST_AST, $2); $$->setLoc(@1); }
    | '[' target_list2 ']'            { $$ = NODE(TARGET_LIST_AST, $2); $$->setLoc(@1); }
    | '[' target_list2 ',' ']'        { $$ = NODE(TARGET_LIST_AST, $2); $$->setLoc(@1); }
    | '(' target_list1 ')'            { $$ = $2; }
    ;    

target_list1 :
      target
    | target ','                      { $$ = NODE(TARGET_LIST_AST, $1); $$->setLoc(@1); }
    | target_list2                    { $$ = NODE(TARGET_LIST_AST, $1); $$->setLoc(@1); }
    | target_list2 ','                { $$ = NODE(TARGET_LIST_AST, $1); $$->setLoc(@1); }
    ;

target_list2 :
      target ',' target               { $$ = new AST_Vect(1, $1); $$->push_back($3); }
    | target_list2 ',' target         { $$ = $1; $$->push_back($3); }
    ;

target :
      var_ref
    | typed_identifier
    | primary '[' tuple_expression ']'    {AST_Ptr n = NODE(NEXT_VALUE_AST); n->setLoc(@4); $$ = NODE(SUBSCRIPT_ASSIGN_AST, makeId("__setitem__", @1), $1, $3, n); $$->setLoc(@1); } 
    | primary '[' ':' ']'   {AST_Ptr n = NODE(NEXT_VALUE_AST); n->setLoc(@4); $$ = NODE(SLICE_ASSIGN_AST, makeId("__setslice__", @1), $1, LEAF(INT_LITERAL_AST, "0", @2), LEAF(INT_LITERAL_AST, MAX_INT, @2), n); $$->setLoc(@1); } 
    | primary '[' ':' expression ']' {AST_Ptr n = NODE(NEXT_VALUE_AST); n->setLoc(@4); $$ = NODE(SLICE_ASSIGN_AST, makeId("__setslice__", @1), $1, LEAF(INT_LITERAL_AST, "0", @2), $4, n); $$->setLoc(@1); } 
    | primary '[' expression ':' ']' {AST_Ptr n = NODE(NEXT_VALUE_AST); n->setLoc(@4); $$ = NODE(SLICE_ASSIGN_AST, makeId("__setslice__", @1), $1, $3, LEAF(INT_LITERAL_AST, MAX_INT, @2),  n); $$->setLoc(@1); } 
    | primary '[' expression ':' expression ']'{AST_Ptr n = NODE(NEXT_VALUE_AST); n->setLoc(@4); $$ = NODE(SLICE_ASSIGN_AST, makeId("__setslice__", @1), $1, $3, $5, n); $$->setLoc(@1); } 
    ;

right_side :
      tuple_expression
    | assign_statement
    ;
            /***** TYPEDEF *****/

formals_list :
    formals_list1         { $$ = NODE(FORMALS_LIST_AST, $1); $$->setLoc(@1); }
  ;

/* formals_list builder. Return type AST_Vect */
formals_list1 :
    id                          { $$ = new AST_Vect(1, $1); }
  | typed_id                    { $$ = new AST_Vect(1, $1); }
  | formals_list1 ',' id        { $$ = $1; $$->push_back($3); }
  | formals_list1 ',' typed_id  { $$ = $1; $$->push_back($3); }
  ;

typed_id :
    id TYPEASSIGN type      { $$ = NODE(TYPED_ID_AST, $1, $3); $$->setLoc(@2); }
  ;

type :
      id                                { AST_Ptr n = NODE(TYPE_LIST_AST); n->setLoc(@1);
                                          $$ = NODE(TYPE_AST, $1, n); $$->setLoc(@1); }
    | type_id
    | type_var
    | '(' ')' ARROW type                { AST_Ptr n = NODE(TYPE_LIST_AST); n->setLoc(@1);
                                          $$ = NODE(FUNCTION_TYPE_AST, $4, n); $$->setLoc(@1); }
    | '(' type_list ')' ARROW type      { $$ = NODE(FUNCTION_TYPE_AST, $5, $2); $$->setLoc(@1); }
    ;

/* Used to declare type_ids of the form "list of [int, int, tuple,..]" */
type_id :
      id OF '[' type_list ']'   { $$ = NODE(TYPE_AST, $1, $4); $$->setLoc(@1); }
    | id OF type        { AST_Ptr n = NODE(TYPE_LIST_AST, $3); n->setLoc(@1); 
                    $$ = NODE (TYPE_AST, $1, n); $$->setLoc(@1); }
    ;

type_var :
      TYPEVAR                   { $$ = LEAF(TYPE_VAR_AST, *$1, @1); }
    ;

/* TYPE_LIST_AST node containing a list of type_ids. Used to construct type ids.*/
type_list :
      type_list0                { $$ = NODE (TYPE_LIST_AST, $1); $$->setLoc(@1); }
    | type_list0 ','            { $$ = NODE (TYPE_LIST_AST, $1); $$->setLoc(@1); }
    ;

/* Non-empty vector list of types. */
type_list0 :
      type                   { $$ = new AST_Vect(1, $1); }
    | type_list0 ',' type   { $$ = $1; $$->push_back($3); }
    ;

typed_identifier :
      id TYPEASSIGN type               { $$ = NODE (TYPED_ID_AST, $1, $3); $$->setLoc(@1); }
    ;

                /*** TUPLE LIST ***/
/* Similar to expression_list. Given a comma seperated list of expressions, returns
the corresponding node. If there are two or more elements, or the list ends with ','
creates a tuple node with the expressions as the node's children.
One lone element is not intepreted as a tuple, unless follwed by a comma (1,).*/
tuple_expression :
      tuple_expression1     { $$ = NODE(TUPLE_AST, $1); $$->setLoc(@1); }
    | tuple_expression1 ',' { $$ = NODE(TUPLE_AST, $1); $$->setLoc(@1); }
    | expression            /* Expr not turned into a tuple of len 1 */
    | expression ','        { $$ = NODE(TUPLE_AST, $1); $$->setLoc(@1); }
    ;

tuple_expression1 :
      expression ',' expression             { $$ = new AST_Vect(1, $1); $$->push_back($3); }
    | tuple_expression1 ',' expression      { $$ = $1; $$->push_back($3); }
    ;

                /*** EXPRESSION LISTS ***/
expression_list :
      expression_list1
    | expression_list1 ','  { $$ = $1; }
    | /*empty*/         { $$ = new AST_Vect(); }
    ;

expression_list1 :
      expression        { $$ = new AST_Vect(1, $1); }
    | expression_list1 ',' expression
                        { $$ = $1; $$->push_back($3); }
    ;

                /*** EXPRESSIONS ***/
expression:
      and_or_expression IF expression ELSE expression   { $$ = NODE (IF_EXPR_AST, $3, $1, $5); $$->setLoc(@1); }
    | and_or_expression
    ;
     
and_or_expression:
      and_or_expression AND and_or_expression { $$ = NODE (AND_AST, $1, $3); $$->setLoc(@1); }
    | and_or_expression OR and_or_expression  { $$ = NODE (OR_AST, $1, $3); $$->setLoc(@1); }
    | not_expression
    ;

not_expression:
    NOT not_expression    {$$ = NODE (UNOP_AST, makeId("__not__", @1), $2); $$->setLoc(@1); }
    | expression0
    ;

                /*** COMPARE EXPRESSIONS ***/
expression0:
      left_cmpr '>' expression1     { $$ = NODE (COMPARE_AST, makeId("__gt__", @1), $1, $3); $$->setLoc(@1); } 
    | left_cmpr GEQ expression1     { $$ = NODE (COMPARE_AST, makeId("__ge__", @1), $1, $3); $$->setLoc(@1); } 
    | left_cmpr '<' expression1     { $$ = NODE (COMPARE_AST, makeId("__lt__", @1), $1, $3); $$->setLoc(@1); } 
    | left_cmpr LEQ expression1     { $$ = NODE (COMPARE_AST, makeId("__le__", @1), $1, $3); $$->setLoc(@1); } 
    | left_cmpr EQ_OP expression1   { $$ = NODE (COMPARE_AST, makeId("__eq__", @1), $1, $3); $$->setLoc(@1); } 
    | left_cmpr NEQ expression1     { $$ = NODE (COMPARE_AST, makeId("__ne__", @1), $1, $3); $$->setLoc(@1); }
    | left_cmpr IS expression1      { $$ = NODE (COMPARE_AST, makeId("__is__", @1), $1, $3); $$->setLoc(@1); }
    | left_cmpr IN expression1      { $$ = NODE (COMPARE_AST, makeId("__in__", @1), $1, $3); $$->setLoc(@1); }
    | left_cmpr IS_NOT expression1  { $$ = NODE (COMPARE_AST, makeId("__isnot__", @1), $1, $3); $$->setLoc(@1); }
    | left_cmpr NOT_IN expression1  { $$ = NODE (COMPARE_AST, makeId("__notin__", @1), $1, $3); $$->setLoc(@1); }
    | expression1
    ;

left_cmpr:
      left_cmpr '>' expression1     { $$ = NODE (LEFT_COMPARE_AST, makeId("__gt__", @1), $1, $3); $$->setLoc(@1); } 
    | left_cmpr GEQ expression1     { $$ = NODE (LEFT_COMPARE_AST, makeId("__ge__", @1), $1, $3); $$->setLoc(@1); } 
    | left_cmpr '<' expression1     { $$ = NODE (LEFT_COMPARE_AST, makeId("__lt__", @1), $1, $3); $$->setLoc(@1); } 
    | left_cmpr LEQ expression1     { $$ = NODE (LEFT_COMPARE_AST, makeId("__le__", @1), $1, $3); $$->setLoc(@1); } 
    | left_cmpr EQ_OP expression1   { $$ = NODE (LEFT_COMPARE_AST, makeId("__eq__", @1), $1, $3); $$->setLoc(@1); } 
    | left_cmpr NEQ expression1     { $$ = NODE (LEFT_COMPARE_AST, makeId("__ne__", @1), $1, $3); $$->setLoc(@1); }
    | left_cmpr IS expression1      { $$ = NODE (LEFT_COMPARE_AST, makeId("__is__", @1), $1, $3); $$->setLoc(@1); }
    | left_cmpr IN expression1      { $$ = NODE (LEFT_COMPARE_AST, makeId("__in__", @1), $1, $3); $$->setLoc(@1); }
    | left_cmpr IS_NOT expression1  { $$ = NODE (LEFT_COMPARE_AST, makeId("__isnot__", @1), $1, $3); $$->setLoc(@1); }
    | left_cmpr NOT_IN expression1  { $$ = NODE (LEFT_COMPARE_AST, makeId("__notin__", @1), $1, $3); $$->setLoc(@1); }
    | expression1
    ;

expression1 :
      expression1 '+' expression1     { $$ = NODE (BINOP_AST, makeId("__add__", @1), $1, $3); $$->setLoc(@1); }
    | expression1 '-' expression1     { $$ = NODE (BINOP_AST, makeId("__sub__", @1), $1, $3); $$->setLoc(@1); }
    | expression1 '*' expression1     { $$ = NODE (BINOP_AST, makeId("__mul__", @1), $1, $3); $$->setLoc(@1); }
    | expression1 '/' expression1     { $$ = NODE (BINOP_AST, makeId("__floordiv__", @1), $1, $3); $$->setLoc(@1); }
    | expression1 '%' expression1     { $$ = NODE (BINOP_AST, makeId("__mod__", @1), $1, $3); $$->setLoc(@1); }
    | factor
    ;


factor :
      '-' factor    {$$ = NODE (UNOP_AST, makeId("__neg__", @1), $2); $$->setLoc(@1); }
    | '+' factor    {$$ = NODE (UNOP_AST, makeId("__pos__", @1), $2); $$->setLoc(@1); }
    | factor1
    ;

factor1 :
      primary POW factor           { $$ = NODE (BINOP_AST, makeId("__pow__", @1), $1, $3); $$->setLoc(@1); }
    | primary
    ;

                /*** PRIMARY EXPRESSIONS ***/

primary :
      INT               { $$ = LEAF(INT_LITERAL_AST, *$1, @1); }
    | string_literal
    | var_ref
    | special_id
    | primary '(' expression_list ')' { $3->insert($3->begin(), $1); $$ = NODE(CALL_AST, $3); }
    | type_id '(' expression_list ')' { $3->insert($3->begin(), $1); $$ = NODE(CALL_AST, $3); }
    | '(' tuple_expression ')' {$$ = $2; }
    | '(' ')' { $$ = NODE (TUPLE_AST); $$->setLoc(@1); }
    | '[' expression_list ']' {$$ = NODE (LIST_DISPLAY_AST, $2); $$->setLoc(@1); }
    | '{' pair_list '}'       {$$ = NODE (DICT_DISPLAY_AST, $2); $$->setLoc(@1); }
    | primary '[' tuple_expression ']'     {$$ = NODE(SUBSCRIPT_AST, makeId("__getitem__", @1), $1, $3); $$->setLoc(@1); } 
    | primary '[' ':' ']'            {$$ = NODE(SLICE_AST, makeId("__getslice__", @1), $1, LEAF(INT_LITERAL_AST, "0", @2), LEAF(INT_LITERAL_AST, MAX_INT, @2)); $$->setLoc(@1); } 
    | primary '[' ':' expression ']' {$$ = NODE(SLICE_AST, makeId("__getslice__", @1), $1, LEAF(INT_LITERAL_AST, "0", @2), $4); $$->setLoc(@1); } 
    | primary '[' expression ':' ']' {$$ = NODE(SLICE_AST, makeId("__getslice__", @1), $1, $3, LEAF(INT_LITERAL_AST, MAX_INT, @2)); $$->setLoc(@1); } 
    | primary '[' expression ':' expression ']' {$$ = NODE(SLICE_AST, makeId("__getslice__", @1), $1, $3, $5); $$->setLoc(@1); } 
    ;

string_literal :
      string_literal STRING   { $$ = $1; $$->appendText(*$2); }
    | STRING                  { $$ = LEAF(STRING_LITERAL_AST, *$1, @1); }
    ;

id : ID                { $$ = makeId(*$1, @1); }
   ;

special_id :
      NONE          { $$ = LEAF(NONE_AST, "", @1); }
    | TRUE          { $$ = LEAF(TRUE_AST, "", @1); }
    | FALSE         { $$ = LEAF(FALSE_AST, "", @1); }
    ;

var_ref : 
      id
    | primary '.' id    { $$ = NODE(ATTRIBUTEREF_AST, $1, $3); }
    ;

pair_list :
      pair_list1
    | pair_list1 ','    { $$ = $1; }
    | /*[> Empty pair_list <] */    { $$ = new AST_Vect(); }
    ;

pair_list1 :
      pair              { $$ = new AST_Vect(1, $1); }
    | pair_list1 ',' pair   { $$ = $1; $$->push_back($3); }

    ;

pair : expression ':' expression   { $$ = NODE(PAIR_AST, $1, $3); }
    ;
%%

bool debugParser = 0;

#include "apyc-lexer.cc"
int in_func = 1;
int in_loop = 2;

void
postprocess (AST_Ptr node, int state) {
    if (node->arity() == 0) {
        return;
    }
    for_iter (child_ptr, *node) {
        AST_Ptr child = *child_ptr;
        const char *name = child->astTypeName();
        if (strcmp(name, "for") == 0
         || strcmp(name, "while") == 0) {
            postprocess(child, state | in_loop);
        } else if (strcmp(name, "def") == 0) {
            postprocess(child, state | in_func);
        } else if (strcmp(name, "break") == 0
                || strcmp(name, "continue") == 0) {
            if ((state & in_loop) != in_loop) {
                yyerror("break or continue found outside loop");
            }
        } else if (strcmp(name, "return") == 0) {
            if ((state & in_func) != in_func) {
                yyerror("return found outside function");
            }
        } else {
            postprocess(child, state);
        }
    }
}

void
parse (FILE* f, const string& name)
{
    if (debugParser)
        yydebug = 1;
    initLexer (name);
    theTree = NULL;
    yyparse ();
    if (theTree != NULL) {
        postprocess(theTree, 0);
        theTree->print (cout, 0);
    }
}

static void
yyerror (const char* s)
{
    error (currentLocation (), s);
}

