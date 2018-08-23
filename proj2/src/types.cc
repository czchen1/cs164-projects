/* -*- mode: C++; c-file-style: "stroustrup"; indent-tabs-mode: nil; -*- */

/* types.cc: AST subclasses related to types. */

/* Authors:  YOUR NAMES HERE */

#include <iostream>
#include "apyc.h"
#include <stdio.h>
#include <string.h>

using namespace std;

/***** AST *****/

void 
AST::resolveTypesInner (Decl* context, Unifier& bind){

} 

void
AST::resolveTypesOuter (Decl* context) 
{
    freezeDecls (true);

    freezeDecls (false);
}

/** As for unify, but SUBST has an undefined state if result is
 *  false. */
static bool
unify1 (Type_Ptr t0, Type_Ptr t1, Unifier& subst)
{
    t0 = subst[t0];
    t1 = subst[t1];
    if (t0 == t1)
        return true;
    if (t0->isTypeVariable ()) {
        subst.bind (t0, t1);
        return true;
    }
    subst.bind (t1, t0);
    if (t1->isTypeVariable ())
        return true;
    if (t0->getId () != t1->getId ())
        return false;
    if (t0->numTypeParams () != t1->numTypeParams ())
        return false;
    for (int i = 0; i < t0->numTypeParams (); i += 1) {
        if (!unify1 (t0->typeParam (i), t1->typeParam (i), subst))
            return false;
    }
    return true;
}

bool
unify (Type_Ptr t0, Type_Ptr t1, Unifier& subst)
{
    Unifier s = subst;
    if (unify1 (t0, t1, s)) {
        subst.swap (s);
        return true;
    }
    return false;
}

bool
unifies (Type_Ptr t0, Type_Ptr t1)
{
    Unifier s;
    return unify (t0, t1, s);
}


/*****   Typed_Tree ***/

Type_Ptr
Typed_Tree::computeType ()
{
    return newTypeVar ();
}

/*****    TYPE    *****/

Type_Ptr
Type::getType ()
{
    error (this, "a type may not be used as a value in this dialect");
    return newTypeVar ();
}

AST_Ptr
Type::getId ()
{
    return NULL;
}


int
Type::numParams ()
{
    return -1;
}

Type_Ptr
Type::returnType ()
{
    return NULL;
}

Type_Ptr
Type::paramType (int k)
{
    throw range_error ("type has no parameters");
}

int
Type::numTypeParams ()
{
    return -1;
}

Type_Ptr
Type::typeParam (int k)
{
    throw range_error ("type has no type parameters");
}

Type_Ptr
Type::setTypeParam (int k, Type_Ptr type)
{
    throw range_error ("type has no type parameters");
}
    

Type_Ptr
Type::boundFunctionType ()
{
    return NULL;
}
    
const Environ*
Type::getEnviron ()
{
    return theEmptyEnviron;
}

bool
Type::isTypeVariable ()
{
    return false;
}

AST_Ptr
Type::replaceBindings (const Unifier& subst)
{
    return AST::replaceBindings (subst);
}

AST_Ptr
Type::replaceBindings (const Unifier& subst, AST_Set& visiting)
{
    Type_Ptr result = subst[this];
    if (result->isTypeVariable ())
        return result;
    result->AST::replaceBindings (subst, visiting);
    return result;
}

/** Add mappings to FRESHMAP such that applying FRESHMAP to the nodes
 *  in TREE results in a tree in which all type variables are replaced by fresh 
 *  ones, unused elesewhere.  Assumes FRESHMAP initially records previously
 *  converted nodes.  When a node is reached circularly, it will be
 *  mapped to itself; a second pass(freshed2) must be used to replace it.
 *  Returns the mapping of TREE. */
static Type_Ptr
freshen1 (Type_Ptr type, Type_Map& freshMap)
{
    auto old = freshMap.emplace (type, type);
    if (!old.second)
        return old.first->second;

    Type_Ptr result;
    if (type->isTypeVariable ())
        result = newTypeVar ();
    else {
        bool changed;
        AST_Vect newKids;

        changed = false;
        for (int i = 0; i < type->numTypeParams (); i += 1) {
            Type_Ptr c = type->typeParam (i), c0 = c;
            c = freshen1 (c, freshMap);
            newKids.push_back (c);
            changed |= (c != c0);
        }

        if (!changed) 
            result = type;
        else if (type->arity () == type->numTypeParams ())
            result = NODE (type->typeIndex (), newKids)->asType ();
        else
            result =
                NODE (type->typeIndex (), type->getId (), newKids)->asType ();
    }
    freshMap[type] = result;
    return result;
}

/** Return a version of TREE with all nodes replaced as indicated by
 *  FRESHMAP.  Assumes that subtrees mapped to themselves in FRESHMAP are
 *  fully processed. */
static Type_Ptr
freshen2 (Type_Ptr type0, Type_Map& freshMap)
{
    Type_Ptr type = freshMap[type0];
    if (type == type0 || type == nullptr)
        return type0;

    for (int i = 0; i < type->numTypeParams (); i += 1)
        type->setTypeParam (i, freshen2 (type->typeParam (i), freshMap));
    
    return type;
}

Type_Ptr
freshen (Type_Ptr tree)
{
    Type_Ptr type = tree->asType ();

    Type_Map freshMap;

    type = freshen1 (type, freshMap);

    return freshen2 (type, freshMap);
}

void
freshen (Type_Vect& types)
{
    Type_Map freshMap;

    for (auto& type : types)
        type = freshen1 (type, freshMap)->asType ();
    for (auto& type : types)
        type = freshen2 (type, freshMap)->asType ();
}

Type_Ptr
makeFuncType (int n)
{
    AST_Vect params;
    for (int i = 0; i <= n; i += 1)
        params.push_back(newTypeVar ());
    return NODE (FUNCTION_TYPE_AST, params)->asType ();
}

bool
makeFuncType (int n, Type_Ptr type, Unifier& subst)
{
    type = subst[type];
    if (!type->isTypeVariable ())
        return (type->isFunctionType () && type->numParams () == n);
    unify (type, makeFuncType (n), subst);
    return true;
}

Type_Ptr
makeMethodType (int n, Decl* clas)
{
    AST_Vect params;
    params.push_back (newTypeVar ());
    params.push_back (clas->asGenericType ());
    for (int i = 1; i < n; i += 1)
        params.push_back(newTypeVar ());
    return NODE (FUNCTION_TYPE_AST, params)->asType();
}

/*****  TYPE VARIABLES *****/

class Type_Var_AST : public Type {
    friend Type_Ptr newTypeVar(const string&);

    bool isTypeVariable () override {
        return true;
    }

    void print (AST_Printer& printer) override {
        printer.os << "(type_var " << location () << " " << text ();
        if (getDecl () != nullptr)
            printer.os << " " << getDecl ()->getIndex ();
        printer.os << ")";
    }

    string text () const override {
        return _name;
    }
           
    /** A previously unused type variable whose name is NAME. */
    static Type_Ptr newTypeVar (const string& name);
        
    const Decl_Vect& getDecls () override {
        return _me;
    }

    int getNumDecls () override {
        return _me.size();
    }

    void addDecl (Decl* decl) override {
        _me.push_back (decl);
    }

    void collectDecls (Decl* enclosing) override {
        if (getDecl() != NULL) {
            //cout << "type_var name " << text() << endl;
            return;
        }
        //cout << "type_var name " << text() << endl;
        Decl *d = enclosing->addTypeVarDecl(this);
        if (d == nullptr) error(location(), "TypeVar decl not created");
        this->addDecl(d);


    }

    AST_Ptr resolveSimpleIds (const Environ* env) {
        Decl_Vect defns;
                //error(location(), "hiii2");
        // Node already has a decl attached to it (eg: it is a declaration)
        if (getDecl() != NULL) {
            return getDecl()->getAst();
        }
        env->find (this->text(), defns);
        for (Decl* c : defns) {
            this->addDecl(c);
            return c->getAst();
        }
        error(location(), "type var not declared: %s", this->text().c_str());
        return NULL;
    }


    Decl_Vect _me;
    string _name;
};

static int lastTypeVarUid = 0;

Type_Ptr
Type_Var_AST::newTypeVar (const string& name)
{
    Type_Var_AST* result = new Type_Var_AST;
    result->_name = name;
    return result;
}

Type_Ptr
newTypeVar (const string& name)
{
    return Type_Var_AST::newTypeVar (name);
}

Type_Ptr
newTypeVar ()
{
    lastTypeVarUid += 1;
    Type_Ptr result = newTypeVar ("$#" + to_string (lastTypeVarUid));
    result->addDecl (makeTypeVarDecl (result));
    return result;
}




/***** FUNCTION TYPES *****/

class Function_Type_AST: public Type {
protected:

    /** For function types, returns a unique artificial id node. */
    AST_Ptr getId () override {
        static AST_Ptr id = makeId ("->", 0);
        return id;
    }

    bool isFunctionType () override {
        return true;
    }

    int numParams () override {
        return arity () - 1;
    }

    Type_Ptr paramType (int k) override {
        return child (k + 1)->asType ();
    }

    Type_Ptr returnType () override {
        return child (0)->asType ();
    }

    int numTypeParams () override {
        return arity ();
    }

    Type_Ptr typeParam (int k) override {
        return child (k)->asType ();
    }

    Type_Ptr setTypeParam (int k, Type_Ptr type) override {
        return setChild (k, type)->asType ();
    }

    Type_Ptr boundFunctionType () override {
        if (numParams () == 0)
            return NULL;
        Type_Ptr result = NODE (FUNCTION_TYPE_AST, returnType ())->asType ();
        for (int i = 1; i < numParams (); i += 1)
            ADD (result, paramType (i));
        return result;
    }

    FACTORY (Function_Type_AST);

};

EXEMPLAR (Function_Type_AST, FUNCTION_TYPE_AST);


/***** CLASSES *****/

class ClassType_AST: public Type {
protected:

    FACTORY (ClassType_AST);

    AST_Ptr getId () override {
        Decl* me = getDecl ();
        if (me == nullptr)
            return child (0);
        else
            return me->getId ();
    }

    int numTypeParams () override {
        return arity () - 1;
    }

    Type_Ptr typeParam (int k) override {
        return child (k + 1)->asType ();
    }

    Type_Ptr setTypeParam (int k, Type_Ptr type) override {
        return setChild (k + 1, type)->asType ();
    }

    const Decl_Vect& getDecls () override {
        return child (0)->getDecls ();
    }

    void addDecl (Decl* decl) override {
        child (0)->addDecl (decl);
    }

    void collectDecls (Decl* enclosing) override {
        for (size_t i = 1; i < this->children().size(); ++i) {
            this->child(i)-> collectDecls (enclosing);
        }
    }

    AST_Ptr resolveSimpleIds (const Environ* env) override {
        for (int i = 1; i < arity(); i++) {
            setChild(i, child(i)->resolveSimpleIds(env));
        }
        return this;
    }

    void resolveSimpleTypeIds (const Environ* env) override {
        if (getDecl() != NULL)
            return;
        AST_Ptr id = this->child(0);
        Decl_Vect defns;
        // Look through the immediate environ 
        Decl* d = env->findImmediate(id->text());
        // If nothing found, go through enclosing
        if (d == NULL) {
            env->find (id->text(), defns);
            for (Decl* c : defns) {
                // Ignore class var/method
                if (c->isInstance() || c->isMethod()) {
                    continue;
                }
                else {
                    // The first thing you find is the decl that should be referenced
                    d = c;
                    break;
                }
            }
        }
        if (d != NULL) {
            // Check to see it exists and is a class decl
            if (d->isType()) {
                this->addDecl(d);
            }
            else {
                error(location(), "type identifier not declared as class: %s", id->text().c_str());
            }
        }
        else {
            error(location(), " type identifier not declared: %s", id->text().c_str()); 
        }
        // Go through parameterized types (Id of [T1, T2,..])
        for (size_t i = 1; i < this->children().size(); i+=1) {
            this->child(i)->resolveSimpleTypeIds (env);
        }
    }

    void
    resolveTypesInner (Decl* context, Unifier& bind) override {
        for (size_t i = 1; i < this->children().size(); ++i) {
            this->child(i)->resolveTypesInner (context, bind);
        }
    }
    
};

EXEMPLAR (ClassType_AST, TYPE_AST);



