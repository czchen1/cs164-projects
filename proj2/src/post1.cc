/* -*- mode: C++; c-file-style: "stroustrup"; indent-tabs-mode: nil; -*- */

/* post1.cc: Preliminary modifications of the AST received from phase 1. */

/* Authors:  P. N. Hilfinger */

#include "apyc.h"

using namespace std;

static AST_Ptr reviseTree(AST_Ptr tree, bool classp);
static AST_Ptr reviseTree2(AST_Ptr tree);
static AST_Ptr makeTypedId(AST_Ptr formal);

/** Assuming that PROG is the AST for a program produced by the parser
 *  compiler phase 1 and PRELUDE is the AST for the stsndard prelude
 *  produced by compiler phase 1, returns the result of destructively
 *  applying the following transformations:
 *
 *  1. The statements of PRELUDE are prepended to those of PROG.
 *  2. All "def" nodes that represent methods are converted to
 *     "method" nodes, which have the same children as defs.
 *  3. All missing return types are replaced by fresh anonymous type variables.
 *  4. All formal parameters are given fresh anonymous type variables as formal
 *     types.
 *  5. All "block" and type_list nodes are removed, and their children
 *     transferred to the enclosing node.
 *
 *  Changes 2-5 are changes that could be made in the parser itself, but may
 *  not have been.
 */ 

AST_Ptr
post1 (AST_Ptr prog, AST_Ptr prelude)
{
    AST_Ptr result = NODE (MODULE_AST);
    prelude->add (prog->children ());
    prelude->setLoc (0);
    
    AST_Ptr tree1 = reviseTree(prelude, false);
    return reviseTree2 (tree1);
}


static void sub_slice_assign_call(AST_Ptr sub_slice, AST_Ptr rhs, AST_Ptr enclosing, int i) {
    if (sub_slice->typeIndex () == SUBSCRIPT_ASSIGN_AST || 
            sub_slice->typeIndex () == SLICE_ASSIGN_AST) {
        enclosing->setChild(i, reviseTree2 (sub_slice));
        enclosing->child(i) -> add (rhs);
    }
}
static AST_Ptr compare (AST_Ptr func_id, AST_Ptr arg1, AST_Ptr arg2, Location loc) {
        AST_Ptr newTree = NODE (CALL_AST, func_id, reviseTree2(arg1), reviseTree2(arg2));
        newTree->setLoc (loc);
        return newTree;
}


static AST_Ptr
reviseTree2 (AST_Ptr tree)
{
    if (tree == NULL)
        return NULL;

    // these must be declared before the switch statement
    AST_Ptr funcName, newTree, expr1, expr2, inner_cmp;
    int k;

    switch (tree->typeIndex ()) {
        default:
            for (auto& c : *tree) {
                c = reviseTree2 (c);
            }
            return tree;
        case ASSIGN_AST:
            if (tree->child(0)->typeIndex () == TARGET_LIST_AST) {
                for (int i = 0; i < tree->child(0)->arity(); i++) {
                    sub_slice_assign_call(tree->child(0)->child(i), tree->child(1)->child(i), tree->child(0), i);
                }
            }
            else if (tree->child(0)->typeIndex () == SUBSCRIPT_ASSIGN_AST ||
                tree->child(0)->typeIndex () == SLICE_ASSIGN_AST) {
                tree->setChild(0, reviseTree2 (tree->child(0)));
                tree->child(0) -> add (tree->child(1));
            }
            for (auto& c : *tree) {
                c = reviseTree2 (c);
            }
            return tree;
        case SUBSCRIPT_ASSIGN_AST:
        case SLICE_ASSIGN_AST:
        case SUBSCRIPT_AST:
        case SLICE_AST:
            funcName = tree->child(0);
            newTree = NODE (CALL_AST, funcName);
            newTree->setLoc (tree->location ());
            
            k = 1;
            while (k < tree->arity() && tree->child(k)->typeIndex() != NEXT_VALUE_AST) {
                newTree->add(tree->child(k));
                k += 1;
            }
            return newTree;
        case BINOP_AST:
            funcName = tree->child(0);
            expr1 = reviseTree2(tree->child(1));
            expr2 = reviseTree2(tree->child(2));

            newTree = NODE (CALL_AST, funcName, expr1, expr2);
            newTree->setLoc (tree->location());
            return newTree;
        case UNOP_AST:
            funcName = tree->child(0);
            expr1 = reviseTree2(tree->child(1));

            newTree = NODE (CALL_AST, funcName, expr1);
            newTree->setLoc (tree->location());
            return newTree;
        case LEFT_COMPARE_AST:
        case COMPARE_AST:
            /* Deals with nested compares and left compares. Turns into and's of boolean expr. */
            inner_cmp = tree->child(1);
            if (inner_cmp->typeIndex() == LEFT_COMPARE_AST) {
                if (inner_cmp->child(1)->typeIndex() == LEFT_COMPARE_AST) {
                    expr1 = reviseTree2(tree->child(1));
                }
                else {
                    expr1 = compare(inner_cmp->child(0), inner_cmp->child(1),
                            inner_cmp->child(2), tree->location());
                }
                expr2 = compare(tree->child(0), inner_cmp->child(2), tree->child(2), tree->location());
                newTree = NODE(AND_AST, expr1, expr2);
            }
            else {
                newTree = compare(tree->child(0), tree->child(1), tree->child(2), tree->location());
            }
            return newTree;
    }
}

/** Perform phase 1 modifications (see phase1 above) on TREE,
 *  returhing modified tree. CLASSP is true iff TREE is a child
 *  of a class node block. */
static AST_Ptr
reviseTree (AST_Ptr tree, bool classp)
{
    if (tree == NULL)
        return NULL;

    switch (tree->typeIndex ()) {
    default:
        for (auto& c : *tree) {
            c = reviseTree (c, false);
        }
        break;

    case CLASS_AST:
        for (auto& c : *tree) {
            c = reviseTree (c, true);
        }
        break;

    case BLOCK_AST:
        for (auto& c : *tree) {
            c = reviseTree (c, classp);
        }
        break;

    }
        
    switch (tree->typeIndex ()) {
    default:
        return tree;

    case IF_AST:
    case WHILE_AST:
        if (tree->arity () < 3) {
            tree->add (NODE (STMT_LIST_AST));
        }
        return tree;

    case FOR_AST:
        if (tree->arity () < 4) {
            tree->add (NODE (STMT_LIST_AST));
        }
        return tree;

    case DEF_AST: {
        if (tree->arity () == 2 
            || tree->child (tree->arity () - 1)->typeIndex () == BLOCK_AST
            || !tree->child (2)->isType ()
            || (classp && tree->typeIndex () != METHOD_AST)) {

            int k;
            AST_Ptr newTree = NODE(classp ? METHOD_AST : DEF_AST, 
                                   tree->child (0), tree->child(1));
            newTree->setLoc (tree->location());

            if (tree->arity () == 2 || !tree->child (2)->isType ()) {
                newTree->add (newTypeVar ());
                k = 2;
            } else {
                newTree->add (tree->child (2));
                k = 3;
            }

            while (k < tree->arity ()) {
                if (tree->child (k)->typeIndex () == BLOCK_AST) {
                    newTree->add (tree->child (k)->children ());
                } else {
                    newTree->add (tree->child (k));
                }
                k += 1;
            }
            
            return newTree;
        }
        return tree;
    }

    case CLASS_AST: case FUNCTION_TYPE_AST: case TYPE_AST: {
        AST_Ptr last = tree->child (tree->arity () - 1);
        if (last->typeIndex () == BLOCK_AST 
            || last->typeIndex () == TYPE_LIST_AST) {
            AST_Ptr newTree = NODE(tree->typeIndex ());
            newTree->setLoc (tree->location ());
            for (int k = 0; k < tree->arity () - 1; k += 1) {
                newTree->add (tree->child (k));
            }
            newTree->add (last->children ());
            return newTree;
        }
        return tree;
    }

    case FORMALS_LIST_AST:
        for (auto& c : *tree) {
            c = makeTypedId (c);
        }
        return tree;
        
    }

}

/** Return FORMAL (an id or typed id) as a typed id.  This is the
 *  identity on TYPE_ID nodes.  It returns a TYPED_ID with a fresh
 *  type variable for plain ID nodes. */
static AST_Ptr
makeTypedId (AST_Ptr formal)
{
    switch (formal->typeIndex ()) {
    default:
        return formal;
    case ID_AST:
        return NODE (TYPED_ID_AST, formal, newTypeVar ());
    }
}

