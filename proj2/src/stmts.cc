/* -*- mode: C++; c-file-style: "stroustrup"; indent-tabs-mode: nil; -*- */

/* stmts.cc: AST subclasses related to statements and modules. */

/* Authors:  YOUR NAMES HERE */

#include <iostream>
#include "apyc.h"
#include "ast.h"
#include <stdio.h>
#include <string.h>

using namespace std;

/*****   MODULE    *****/

/** A module, representing a complete source file. */
class Module_AST : public AST_Tree {
public:

    Location location () const override {
        return 0;
    }

    /** Top-level semantic processing for the program. */
    AST_Ptr doOuterSemantics () override {
        //outerEnviron = new Environ (nullptr);
        mainModule = makeModuleDecl (makeId ("__main__", 0));
        outerEnviron = mainModule->getEnviron();
        for (auto& c : *this) {
            //error(location(),"child of module: %s", c->astTypeName ());
            c = c->doOuterSemantics ();
        }
        return this;
    }
    FACTORY (Module_AST);

};

EXEMPLAR (Module_AST, MODULE_AST);


/*****   PRINTLN   *****/

/** A print statement without trailing comma. */
class Println_AST : public AST_Tree {
public:

    FACTORY (Println_AST);

    void
    collectDecls (Decl* enclosing) override {
    }

    void
    resolveTypesOuter (Decl* context) override {
        Unifier bind;
        for(iterator c = begin(); c != end(); c++){
            (*c)->resolveTypesInner(context, bind);
        }
    }
    void
    resolveTypesInner (Decl* context, Unifier& bind) override {  // MIGHT NEED TO CHANGE PRINT.  What am i supposed to enforce?
        for(iterator c = begin(); c != end(); c++){
            (*c)->resolveTypesInner(context, bind);
        }
    }

};

EXEMPLAR (Println_AST, PRINTLN_AST);


/***** STMT_LIST *****/

/** A list of statements. */
class StmtList_AST : public AST_Tree {
protected:

    FACTORY (StmtList_AST);

    AST_Ptr doOuterSemantics () override {
        for (auto& c : *this) {
            c = c->doOuterSemantics ();
        }
        return this;
    }
    
    void
    resolveTypesOuter (Decl* context) override {
        Unifier bind;
        for(iterator c = begin(); c != end(); c++){
            (*c)->resolveTypesInner(context, bind);
        }
    } 
    void
    resolveTypesInner (Decl* context, Unifier& bind) override {
        for(iterator c = begin(); c != end(); c++){
            (*c)->resolveTypesInner(context, bind);
        }
    }

};

EXEMPLAR (StmtList_AST, STMT_LIST_AST);


/** ASSIGN Statement **/
class Assign_AST : public AST_Tree {

    FACTORY (Assign_AST);

    Type_Ptr
    computeType () override {
        return child(0)->getType();
    }

    void
    collectDecls (Decl* enclosing) override {
        AST_Ptr lhs = this->child(0);
        AST_Ptr rhs = this->child(1);
        lhs -> collectDecls(enclosing);
        if (rhs->typeIndex() == ASSIGN_AST) {
            rhs->collectDecls(enclosing);
        }
        // TODO: REMOVE ME + fix properly
/*        if (child(0)->typeIndex() == CALL_AST) {*/
            //if (child(0)->child(0)->text().compare("__setitem__")==0) {
                //child(0)->add(child(1));
            //}
        /*}*/
    }

    void 
    resolveTypesOuter (Decl* context) override {
        Unifier bind;
        AST_Ptr left = child(0);
        AST_Ptr right = child(1); 
        left->resolveTypesInner(context, bind);
        right->resolveTypesInner(context, bind);
        Type_Ptr l = left->getType();
        Type_Ptr r = right->getType();
        if (l==NULL || r==NULL) return;

        // Commented out stuff is for debugging
        // TODO: remove me in the end
/*        l->print(cout, 0);*/
        //cout << endl;
        //r->print(cout, 0);
        /*cout << endl;*/
        bool t = unify(l, r, bind); 

        if (!t) {
            error(location(), "unification failed");
            return;
        }
        //bind.print();
        left->replaceBindings(bind);
    }

    void
    resolveTypesInner (Decl* context, Unifier &bind) override {
  
        AST_Ptr left = child(0);
        AST_Ptr right = child(1); 
        left->resolveTypesInner(context, bind);
        right->resolveTypesInner(context, bind);
 
        bool t = unify(left->getType(), right->getType(), bind); 

        //bind.print();
        if (!t) {
            error(location(), "unification failed");
            return;
        }
        left->replaceBindings(bind);
    } 
}; 


EXEMPLAR (Assign_AST, ASSIGN_AST);

class Def_AST : public AST_Tree {
    FACTORY(Def_AST);

    void collectDecls (Decl* enclosing) override {
        // (function N id formals_list return_type stmt_list*)
        AST_Ptr id = this->child(0);
        AST_Ptr params_list = this->child(1);
        //  func decl -> id, k (param_list size)
        Decl* d = enclosing -> addDefDecl(id, params_list->arity());
        if (d == nullptr) error(location(), "null func decl");
        id -> addDecl(d);
        d -> getType() -> setTypeParam(0, this->child(2)->asType());
        child(2)->collectDecls(d);
        // parameters list
        int k = 1;
        for (auto& param : *params_list) {
            Type_Ptr type = param->child(1)->asType();
            Decl* p = d -> addParamDecl(param->child(0), k, type);
            d -> addParam(p); // Added for paramdecl to func decl
            if (p == nullptr) error(location(), "null param decl");
            d -> getType() -> setTypeParam (k, type);
            param -> child(0) -> addDecl(p); // Added paramdecl to id node
            // Create decl for typevar
            param->child(1)->collectDecls(d);
            //cout << "type_var name " << param->child(1)->text() << endl;
            k++;
        }
        // block stmts
        for (size_t i = 3; i < this->children().size(); i+=1) { 
            this->child(i)->collectDecls(d);
        }
    }


    AST_Ptr resolveSimpleIds (const Environ* env) override {
         /*ResolveSimpleIds resolves ids within the function block;*/
         /*Environment defines decls within the function (including params)*/
 
        Decl* defDecl = this->child(0)->getDecl();
        //error(location(), defDecl->getName().c_str());
        for (auto& c : *this) {
            c = c->resolveSimpleIds (defDecl->getEnviron());
        }
        return this;
    }


    void  
    resolveTypesOuter (Decl* context) override {


        Unifier bind;
        for(iterator c = begin(); c != end(); c++){
            (*c)->resolveTypesInner(child(0)->getDecl(), bind);
        }


    }
    void
    resolveTypesInner (Decl* context, Unifier& bind) override {
        // error(location(), "abcdes");
        for(iterator c = begin(); c != end(); c++){
            (*c)->resolveTypesInner(child(0)->getDecl(), bind);
        }
        //error(location(), "abcde2");

    }

};
EXEMPLAR(Def_AST, DEF_AST);

// ADDED, TODO: what is the difference btwn this and def_AST
class Method_AST : public Def_AST {

    FACTORY(Method_AST);

    void
    resolveTypesInner (Decl* context, Unifier& bind) override {
        if(child(1)->arity() == 0){
            error(location(), "need self");
        }

        for(iterator c = begin(); c != end(); c++){
            (*c)->resolveTypesInner(child(0)->getDecl(), bind);
        }
    }

};
EXEMPLAR(Method_AST, METHOD_AST);


class Class_AST : public AST_Tree {

    FACTORY(Class_AST);

    void collectDecls (Decl* enclosing) override {
        AST_Ptr id = this->child(0);
        AST_Ptr type_formals_list = this->child(1);
        Decl* d = enclosing -> addClassDecl(id, type_formals_list);
        id -> addDecl(d);

        for (auto& type_var : *type_formals_list) {
            Decl* p = d -> addTypeVarDecl(type_var);
            if (p == nullptr) error(location(), "null class typevar decl");
            type_var -> addDecl(p);
        }

        setBuiltinDecl(d);
        for (size_t i = 2; i < this->children().size(); i+=1) { 
            this->child(i)->collectDecls(d);
        }
    }


    AST_Ptr resolveSimpleIds (const Environ* env) override {
        Decl* classDecl = this->child(0)->getDecl();
        // error(location(), classDecl->getName().c_str());
        for (auto& c : *this) {
            // error(location(), c->astTypeName());
            c = c->resolveSimpleIds (classDecl->getEnviron());
        }
        return this;
    }
    void  
    resolveTypesOuter (Decl* context) override {
        Unifier bind;
        for(iterator c = begin(); c != end(); c++){
            (*c)->resolveTypesInner(context, bind);
        }
    }
};

EXEMPLAR(Class_AST, CLASS_AST);

class While_AST : public AST_Tree {

    FACTORY (While_AST);
    void  
    resolveTypesOuter (Decl* context) override {
        Unifier bind;
        resolveTypesInner(context, bind);
    }
    void
    resolveTypesInner (Decl* context, Unifier& bind) override {
        for(iterator c = begin(); c != end(); c++){
            (*c)->resolveTypesInner(context, bind);
        }
        if (!child(0)->setType(boolDecl->asType(), bind)) {
            error(location(), "while condition not boolean");
        }
    }

};
EXEMPLAR (While_AST, WHILE_AST);

class Return_AST : public AST_Tree {

    FACTORY (Return_AST);

    void
    resolveTypesInner (Decl* context, Unifier& bind) override {
        for(iterator c = begin(); c != end(); c++){
            (*c)->resolveTypesInner(context, bind);
        }
        for(iterator c = begin(); c != end(); c++){
            (*c)->resolveDeclTypes(bind);
        }
        if(!(unify(context->getType()->returnType(), child(0)->getType(), bind))){
            error(location(), "incorrect type");
        }
        context->replaceBindings(bind);
        setType(child(0)->getType(), bind);
        replaceBindings(bind);
        resolveDeclTypes(bind);

    }

};
EXEMPLAR (Return_AST, RETURN_AST);

class If_AST : public AST_Tree {
    FACTORY (If_AST);
    
    void
    resolveTypesOuter (Decl* context) override {
        Unifier bind;
        resolveTypesInner(context, bind);
    }

    void
    resolveTypesInner (Decl* context, Unifier& bind) override {
        for (iterator c = begin(); c != end(); ++c) {
            (*c)->resolveTypesInner(context, bind);
        }
        if (!child(0)->setType(boolDecl->asType(), bind)) {
            error(location(), "if condition not boolean");
        }
    }
};
EXEMPLAR (If_AST, IF_AST); 

class For_AST : public AST_Tree {
    FACTORY (For_AST);

    void
    collectDecls (Decl* enclosing) override {
        AST_Ptr temp = NODE (CALL_AST, makeId("__choose__", 0), child(1));
        temp = NODE (ASSIGN_AST, child(0), temp);
        child(arity() - 2)->add(temp);   
        
        for (auto& c : *this) {
            c->collectDecls(enclosing);
        }
    }

    void
    resolveTypesOuter (Decl* context) override {
        Unifier bind;
        resolveTypesInner(context, bind);
    }

    void
    resolveTypesInner (Decl* context, Unifier& bind) override {
      //  temp->resolveTypesInner(context, bind);

        for (iterator c = begin(); c != end(); ++c) {
            (*c)->resolveTypesInner(context, bind);
        }

        // ADDED
//        if(!(unify(child(0)->getType(), child(1)->child(0)->getType(), bind))){
//            error(location(), "wrong type");
//        }
//        child(0)->resolveDeclTypes(bind);

        child(arity() - 2)->children().pop_back();
    }
};
EXEMPLAR (For_AST, FOR_AST);

