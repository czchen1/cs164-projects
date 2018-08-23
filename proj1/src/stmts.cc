/* -*- mode: C++; c-file-style: "stroustrup"; indent-tabs-mode: nil; -*- */

/* stmts.cc: AST subclasses related to statements and modules. */

/* Authors:  YOUR NAMES HERE */

#include <iostream>
#include "apyc.h"
#include "ast.h"

using namespace std;

/*****   MODULE    *****/

/** A module, representing a complete source file. */
class Module_AST : public AST_Tree {
public:
    Location location () const {
        return 0;
    }
    FACTORY (Module_AST);
};
EXEMPLAR (Module_AST, MODULE_AST);

class Class_AST : public AST_Tree {
public:
    FACTORY (Class_AST);
};
EXEMPLAR (Class_AST, CLASS_AST);

class Type_Formals_List_AST : public AST_Tree {
public:
    FACTORY (Type_Formals_List_AST);
};
EXEMPLAR (Type_Formals_List_AST, TYPE_FORMALS_LIST_AST);

class Block_AST : public AST_Tree {
public:
    FACTORY (Block_AST);
};
EXEMPLAR (Block_AST, BLOCK_AST);

class Function_Type_AST : public AST_Tree {
public:
    FACTORY (Function_Type_AST);
};
EXEMPLAR (Function_Type_AST, FUNCTION_TYPE_AST);

class Def_AST : public AST_Tree {
public:
    FACTORY (Def_AST);
};
EXEMPLAR (Def_AST, DEF_AST);


/***** STATEMENTS  *****/

class Statement_List_AST : public AST_Tree {
public:
    FACTORY (Statement_List_AST);
};
EXEMPLAR (Statement_List_AST, STMT_LIST_AST);

class Continue_AST : public AST_Leaf {
public:
    FACTORY (Continue_AST);
};
EXEMPLAR (Continue_AST, CONTINUE_AST);

class Break_AST : public AST_Leaf {
public:
    FACTORY (Break_AST);
};
EXEMPLAR (Break_AST, BREAK_AST);

class Return_AST : public AST_Tree {
public:
    FACTORY (Return_AST);
};
EXEMPLAR (Return_AST, RETURN_AST);

class While_AST : public AST_Tree {
public:
    FACTORY (While_AST);
};
EXEMPLAR (While_AST, WHILE_AST);

class For_AST : public AST_Tree {
public:
    FACTORY (For_AST);
};
EXEMPLAR (For_AST, FOR_AST);

class Empty_AST : public AST_Leaf {
public:
    void print (ostream& out, int indent) {
        out << "()";
    }
    FACTORY (Empty_AST)
};
EXEMPLAR (Empty_AST, EMPTY_AST);

/*****   PRINTLN   *****/

/** A print statement without trailing comma. */
class Println_AST : public AST_Tree {
public:
    FACTORY (Println_AST);
};
EXEMPLAR (Println_AST, PRINTLN_AST);

class Print_AST : public AST_Tree {
public:
    FACTORY (Print_AST);
};
EXEMPLAR (Print_AST, PRINT_AST);

class If_AST : public AST_Tree {
public:
    FACTORY (If_AST);
};
EXEMPLAR (If_AST, IF_AST);

class Native_AST : public AST_Tree {
public:
    FACTORY (Native_AST);
};
EXEMPLAR (Native_AST, NATIVE_AST);


/*****  ASSIGN     *****/
class Assign_AST : public AST_Tree {
public:
    FACTORY (Assign_AST);
};
EXEMPLAR (Assign_AST, ASSIGN_AST);

class Target_List_AST : public AST_Tree {
public:
    FACTORY (Target_List_AST);
};
EXEMPLAR (Target_List_AST, TARGET_LIST_AST);

class Subscript_Assign_AST : public AST_Tree {
public:
    FACTORY (Subscript_Assign_AST);
};
EXEMPLAR (Subscript_Assign_AST, SUBSCRIPT_ASSIGN_AST);

class Slice_Assign_AST : public AST_Tree {
public:
    FACTORY (Slice_Assign_AST);
};
EXEMPLAR (Slice_Assign_AST, SLICE_ASSIGN_AST);

class Next_AST : public AST_Tree {
public:
    FACTORY (Next_AST);
};
EXEMPLAR (Next_AST, NEXT_VALUE_AST);

class Typed_Identifier_AST : public AST_Tree {
public:
    FACTORY (Typed_Identifier_AST);
};
EXEMPLAR (Typed_Identifier_AST, TYPED_ID_AST);

class Type_Id_AST : public AST_Tree {
public:
    FACTORY (Type_Id_AST);
};
EXEMPLAR (Type_Id_AST, TYPE_AST);

class Type_List_AST : public AST_Tree {
public:
    FACTORY (Type_List_AST);
};
EXEMPLAR (Type_List_AST, TYPE_LIST_AST);

class Type_Var_AST : public AST_Leaf {
public:
    void print (ostream& out, int indent) {
        out << "(type_var " << location () << " " << this->text () << ")";
    }
    FACTORY (Type_Var_AST);
};
EXEMPLAR (Type_Var_AST, TYPE_VAR_AST);

class Formals_List_AST : public AST_Tree {
public:
    FACTORY (Formals_List_AST);
};
EXEMPLAR (Formals_List_AST, FORMALS_LIST_AST);

