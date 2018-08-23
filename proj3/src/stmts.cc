/* -*- mode: C++; c-file-style: "stroustrup"; indent-tabs-mode: nil; -*- */

/* stmts.cc: AST subclasses related to statements and modules. */

/* Authors:  YOUR NAMES HERE */

#include <iostream>
#include "apyc.h"
#include "ast.h"

using namespace std;

static unordered_map<string, string> builtInNames({
    { "int", "Int" },
    { "list", "List" },
    { "tuple0", "Tuple0" },
    { "tuple1", "Tuple1" },
    { "tuple2", "Tuple2" },
    { "tuple3", "Tuple3" },
    { "str", "Str" },
    { "dict", "Dict" },
    { "bool", "Bool" },
    { "range", "Range" },
});

/*****   MODULE    *****/

/** A module, representing a complete source file. */
class Module_AST : public AST_Tree {
public:

    Location location () const override {
        return 0;
    }

    /** Top-level semantic processing for the program. */
    AST_Ptr doOuterSemantics () override {
        mainModule = makeModuleDecl (makeId ("__main__", 0));
        outerEnviron = mainModule->getEnviron ();
        for (auto& c : *this) {
            c = c->doOuterSemantics ();
        }
        return this;
    }

    void codeGenOuter(Code_Context& context) {
        context << "#include \"runtime.h\"" << endl;
        //context << "#include \"lib/runtime.h\"" << endl;
        context << "using namespace std;" << endl;
        context << "Env global_env;" << endl;
        AST::codeGenOuter (context);
    }

    /* Dummy placeholder for real code. */
    void codeGen(Code_Context& context, bool constructor) {
        context << "char** main_argv;" << endl;
        context << "int main_argc;" << endl;
        context << "int main(int argc, char* argv[])" << endl
                << "{" << endl;
        context.indent ();

        context << "main_argv = argv;" << endl;
        context << "main_argc = argc;" << endl;
        context << "Env env = new Env_Obj (NULL);" << endl;
        for (Decl* d : mainModule->getEnviron()->get_members())
        {
            if (d->isFunc())
            {
                context << "env->set (\"" << d->uniqueName () << "\", ";
                context << "new Closure_Obj <";
                d->getType ()->codeGenDecls (context);
                context << "> (&";
                context << d->uniqueName () << ", env));" << endl;
            }
        }

/*        for (auto c : *this) {*/
            //c->codeGen (context, constructor);
        /*}*/
        int i = 0;
        for (auto c : *this) {
            if (i > 0 && c->arity () > 0 && child(i-1)->arity() == 1 && (child(i-1)->child(0)->typeIndex () == PRINT_AST)
                    && (c->child(0)->typeIndex () == PRINT_AST || c->child(0)->typeIndex () == PRINTLN_AST) && (c->child(0)->arity () > 0)) {
                context << "cout << \" \";" << endl;
            }
            c->codeGen (context, constructor);
            context << ";" << endl;
            i++;
        }

        context.indent (-1);
        context << "}" << endl;
    }

    FACTORY (Module_AST);

};

EXEMPLAR (Module_AST, MODULE_AST);


/*****   PRINTLN   *****/

/** A print statement without trailing comma. */
class Println_AST : public AST_Tree {
public:

    void codeGen(Code_Context& context, bool constructor) override {
        context << "cout << ";
        if (arity () > 0) {
            child (0)->codeGen (context, constructor);
            for (auto c : trimmedAST(*this, 1)) {
                context << " << \" \" << ";
                c->codeGen (context, constructor);
            }
            context << " << ";
        }
        context << " endl";
    }
    FACTORY (Println_AST);
};

EXEMPLAR (Println_AST, PRINTLN_AST);


/*****   PRINT   *****/

/** A print statement with a trailing comma. */
class Print_AST : public AST_Tree {
public:

    void codeGen (Code_Context& context, bool constructor) override {
        context << "cout << ";
        if (arity () > 0) {
            child (0)->codeGen (context, constructor);
            for (auto c : trimmedAST(*this, 1)) {
                context << " << \" \" << ";
                c->codeGen (context, constructor);
            }
            //context << " << \" \"";
        }
    }
    FACTORY (Print_AST);
};

EXEMPLAR (Print_AST, PRINT_AST);


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

    void codeGen (Code_Context& context, bool constructor) {
/*        for (auto c : *this) {*/
            //c->codeGen (context, constructor);
            //context << ";" << endl;
        /*}*/

        int i = 0;
        for (auto c : *this) {
            if (i > 0 && (child(i-1)->typeIndex () == PRINT_AST)
                    && (c->typeIndex () == PRINT_AST || c->typeIndex () == PRINTLN_AST) && (c->arity () > 0)) {
                context << "cout << \" \";" << endl;
            }
            c->codeGen (context, constructor);
            context << ";" << endl;
            i++;
        }

    }

    void codeGenDecls (Code_Context& context) {
    }

};

EXEMPLAR (StmtList_AST, STMT_LIST_AST);

/***** DEF *****/

/** def ID(...): ... */
class Def_AST : public AST_Tree {
protected:

    Decl* getDecl () override {
        return child (0)->getDecl ();
    }

    AST_Ptr getId () override {
        return child (0);
    }

    void freezeDecls (bool frozen) override {
        Decl* me = getDecl ();
        if (me != nullptr)
            me->setFrozen (frozen);
        for (auto c : trimmedAST (*this, 3))
            c->freezeDecls (frozen);
    }

    void collectDecls (Decl* enclosing) override {
        AST_Ptr id = child (0);
        AST_Ptr params = child (1);
        AST_Ptr returnType = child (2);
        Decl* me = enclosing->addDefDecl(child(0), params->arity ());
        id->addDecl (me);
        params->collectDecls (me);
        params->collectTypeVarDecls (me);
        returnType->collectTypeVarDecls (me);
        for (auto c : trimmedAST (*this, 3)) {
            c->collectDecls (me);
        }
    }

    AST_Ptr resolveSimpleIds (const Environ* env) override {
        for (auto& c : *this) {
            c = c->resolveSimpleIds (getDecl ()->getEnviron ());
        }
        return this;
    }

    void resolveTypes (Decl* context, Unifier& subst) override {
        Decl* me = getDecl ();
        Type_Ptr funcType = me->getType ();
        Type_Ptr returnType = child (2)->asType ();
        AST_Ptr formals = child (1);
        getId ()->resolveTypes (me, subst);
        formals->resolveTypes (me, subst);
        if (!unify (funcType->returnType (), returnType, subst))
            error (this, "inconsistent return type");
        for (int i = 0; i < formals->arity (); i += 1) {
            if (!unify (formals->child (i)->getType (),
                        funcType->paramType (i), subst)) {
                error (this, "inconsistent parameter type");
            }
        }
        for (auto c : trimmedAST(*this, 3)) {
            c->resolveTypes (me, subst);
        }
        assert (subst[me->getType ()]->numParams () == formals->arity ());
    }

    void freeTypeVarCheck (const Type_Set& allowed) override {
    }

    virtual void funcName (Code_Context& context) {
        child (0)->codeGenDecls (context);
    }

    void codeGenDecls (Code_Context& context) override {
        Decl* d = getDecl();
        AST_Ptr id = child (0);
        Type_Ptr returnType = child(2)->asType();
        AST_Ptr formals = child (1);
        Decl_Vect decls = d->getEnviron ()->get_members ();


        // Put nested function defintions outside
        for (auto c : trimmedAST (*this, 3)) {
            c->codeGenDecls(context);
        }

        // TODO: template function
        //if (type_formals->arity() > 0) {
            //context << "template <class ";
            //// TODO: commaSeparated?
            //formals->child (0)->codeGen (context, constructor);
            //for (auto c : trimmedAST (*formals, 1)) {
                //context << ", class ";
                //c->codeGen (context, constructor);
            //}
            //context << ">" << endl;
        /*}*/

        returnType->codeGen (context);
        context << " ";
        funcName (context);
        context << " (";
        formals->commaSeparatedCodeGen (context, 0, -1, true);
        if (formals->arity () > 0)
            context << ", ";
        context << "Env parent_env";
        context << ") {" << endl;
        context.indent ();

        context << "Env env = new Env_Obj(parent_env);" << endl;
        for (auto c : *formals)
        {
            context << "env->set (\"" << c->uniqueName () << "\", " << c->uniqueName () << ");" << endl;
        }
        for (Decl* d : getDecl()->getEnviron()->get_members())
        {
            if (d->isFunc())
            {
                context << "env->set (\"" << d->uniqueName () << "\", ";
                context << "new Closure_Obj <";
                d->getType ()->codeGenDecls (context);
                context << "> (&";
                context << d->uniqueName () << ", env));" << endl;
            }
        }
        //codeGenForwardVarDecl (context, getDecl());

        if (child (3)->typeIndex () == NATIVE_AST) {
            context << "return ";
            context << child (3)->child (0)->denotedText ();
            context << "(";
            formals->commaSeparatedCodeGen (context, 0);
            context << ");" << endl;
            context.indent (-1);
            context << "}" << endl;
            return;
        }

        for (auto c : trimmedAST (*this, 3)) {
            c->codeGen (context);
        }
        context << "return static_cast<";
        child(2)->codeGen(context);
        context << "> (new Any_Obj ());" << endl;
        context.indent(-1);
        context << "}" << endl;
    }

    void codeGen (Code_Context& context, bool constructor) override {

    }

    FACTORY (Def_AST);

};


EXEMPLAR (Def_AST, DEF_AST);

/***** METHOD *****/

/**  def ID(...): ... (in class)     */
class Method_AST : public Def_AST {
protected:

    FACTORY (Method_AST);

    void funcName (Code_Context& context) override {
        child (0)->codeGenDecls (context);
        string name = child (0)->text ();
        if (name.compare ("__str__") == 0 || name.compare ("__init__") == 0)
            return;
        else
        context << "_method";
    }

    void collectDecls (Decl* enclosing) override {
        AST_Ptr params = child (1);
        if (params->arity () == 0) {
            error (this, "method must have at least one parameter");
            setChild (1, NODE (FORMALS_LIST_AST,
                               makeId("__self__", location ()),
                               params->children ()));
        }
        Def_AST::collectDecls (enclosing);
    }

    void resolveClassStmtTypes (Decl* context, Unifier& subst,
                                bool doMethods) override {
        if (doMethods)
            resolveTypes (context, subst);
    }

    void codeGenDecls (Code_Context& context) override {
        AST_Ptr params = child (1);
        AST_Vect& children =  params->children ();
        children.erase (children.begin ());
        // TODO: find prettier way to remove first parameter, also check for first param to be self??
        setChild (1, NODE (FORMALS_LIST_AST, children));
        Def_AST::codeGenDecls (context);
    }
};

EXEMPLAR (Method_AST, METHOD_AST);


/***** FORMALS_LIST *****/

/** ... (E1, ...)  */
class FormalsList_AST : public AST_Tree {
protected:

    FACTORY (FormalsList_AST);

    void collectDecls (Decl* enclosing) override {
        int k;
        k = 0;
        for (auto c : *this) {
            AST_Ptr id = c->getId ();
            c->addDecl (enclosing->addParamDecl(id, k));
            k += 1;
        };

        for (auto c : *this) {
            c->collectTypeVarDecls (enclosing);
        };
    }

};

EXEMPLAR (FormalsList_AST, FORMALS_LIST_AST);


/***** CLASS *****/

/** class ID of [...]: ...  */
class Class_AST : public AST_Tree {
protected:

    FACTORY (Class_AST);

    Decl* getDecl () override {
        return child (0)->getDecl ();
    }

    void collectDecls (Decl* enclosing) override {
        AST_Ptr id = child (0);
        string name = id->text ();
        AST_Ptr params = child (1);
        const Environ* env = enclosing->getEnviron ();

        Decl* me = makeClassDecl (id, params);

        if (env->findImmediate (name) != nullptr) {
            error (id, "attempt to redefine %s", name.c_str ());
        } else {
            enclosing->addMember (me);
        }

        setBuiltinDecl (me);

        id->addDecl (me);
        params->collectDecls (me);

        for (auto c : trimmedAST (*this, 2)) {
            c->collectDecls (me);
        }
    }

    AST_Ptr resolveSimpleIds (const Environ* env) override {
        for (auto& c : trimmedAST (*this, 2)) {
            c = c->resolveSimpleIds (getDecl ()->getEnviron ());
        }
        return this;
    }

    void resolveTypes (Decl* context, Unifier& subst) override {
        int errs0 = numErrors ();
        for (auto c : trimmedAST (*this, 2)) {
            c->resolveClassStmtTypes (getDecl (), subst, false);
        }
        if (errs0 == numErrors ())
            replaceBindings (subst);
        for (auto c : trimmedAST (*this, 2)) {
            c->resolveClassStmtTypes (getDecl (), subst, true);
        }
    }

    void freeTypeVarCheck (const Type_Set& allowed) override {
        Type_Set typeParams;
        for (auto c : *child (1)) {
            typeParams.insert (c->asType ());
        }
        for (auto c : trimmedAST (*this, 2)) {
            c->freeTypeVarCheck (typeParams);
        }
    }

    void codeGenDecls (Code_Context& context) override {
        AST_Ptr class_id = child (0);
        AST_Ptr type_formals = child (1);

        // Ignore built in classes in python prelude
        // TODO: clean up builtInNames
        unordered_map<string, string>::const_iterator entryPair = builtInNames.find (class_id -> text ());
        if (entryPair != builtInNames.end ())
            return;

        // Template class

        if (type_formals->arity() > 0) {
            context << "template <class ";
            // TODO: commaSeparated?
            for (auto c : trimmedAST (*type_formals, 1)) {
                context << ", class ";
                c->codeGen (context);
            }
            context << ">" << endl;
            context << "class " << class_id->uniqueName () << "_Obj<";
            type_formals->child (0)->codeGen (context);
            for (auto c : trimmedAST (*type_formals, 1)) {
                context << ", class ";
                c->codeGen (context);
            }
            context << ">" << endl;


            context << "using " << class_id->uniqueName () << " = " << class_id->uniqueName () << "_Obj<";
            type_formals->child (0)->codeGen (context);
            for (auto c : trimmedAST (*type_formals, 1)) {
                context << ", class ";
                c->codeGen (context);
            }
            context << ">" << endl;
            context << "template <class ";
            // TODO: commaSeparated?
            for (auto c : trimmedAST (*type_formals, 1)) {
                context << ", class ";
                c->codeGen (context);
            }
            context << ">" << endl;
        }
        else {
            context << "class " << class_id->uniqueName () << "_Obj;" << endl;
            context << "typedef " << class_id->uniqueName () << "_Obj* " << class_id->uniqueName () << ";" << endl;
        }

        // Class body
        context << "class " << class_id->uniqueName () << "_Obj : public Any_Obj";
        context << " {" << endl << endl;
        context << "public:" << endl << endl;
        context.indent ();

        // Constructor
        // TODO: find a nicer way to deal with __init__
        Decl* init_decl = getDecl ()->getEnviron ()->findImmediate ("__init__");
        int num_params = init_decl->getType ()->numParams ();
        Decl_Vect members = init_decl->getEnviron ()->get_members ();
        // TODO take init's params
        context << class_id->uniqueName () << "_Obj (";

        if (num_params > 1) {
            members[1]->getType ()->codeGen (context);
            context << " " << members[1]->getName ();
        }
        for (int i = 2; i < num_params; i++) {
            context << ", ";
            members[i]->getType ()->codeGenDecls (context);
            context << " " << members[i]->getName ();
        }
        context << ") {" << endl;
        context.indent ();

        // Any non-function definitions are within the constructor
        context << "Env env = new Env_Obj (global_env);" << endl;
        for (auto c : trimmedAST (*this, 2))
            c->codeGen (context, true);

        // Call python's init
        context << "__init__ (";
        if (num_params > 1) {
            context << members[1]->getName ();
        }
        for (int i = 2; i < num_params; i++) {
            context << members[i]->getName ();
            context << ", ";
        }
        if (num_params > 1)
            context << ", ";
        context << "env); " << endl;

        // Create closures
        for (Decl* d : getDecl()->getEnviron()->get_members())
        {
            if (d->isFunc())
            {
                string name = d->getId ()->text ();
                if (name.compare ("__str__") == 0 || name.compare ("__init__") == 0) {
                    continue;
                }
                Type_Ptr t = d->getType ();
                context << d->uniqueName () << " = ";
                context << "new Closure_Method_Obj <";
                //t->codeGenDecls (context);
                t -> child (0)->codeGen (context);
                context << ", ";
                t -> child (1)->codeGen (context);
                 context << "_Obj";
                for (auto c : trimmedAST (*t, 2)) {
                    context << ", ";
                    c->codeGen (context);
                }
                context << "> (&" <<  class_id->uniqueName () << "_Obj::";
                context << d->uniqueName () << "_method, this, env);" << endl;
            }
        }

        context.indent (-1);
        context << "}" << endl << endl;

        // Class method definitions
        for (auto c : trimmedAST (*this, 2))
            c->codeGenDecls (context);

        // Declare class instance variables
        codeGenForwardVarDecl (context, getDecl());

        context << "};" << endl << endl;

        context.indent (-1);
        codeGentypedef (context);
    }

    void codeGentypedef (Code_Context& context) {
        AST_Ptr class_id = child (0);
        AST_Ptr type_formals = child (1);
        if (type_formals->arity() > 0) {

        }


    }

    void codeGen (Code_Context& context, bool constructor) override {
    };


};

EXEMPLAR (Class_AST, CLASS_AST);

/***** TYPE_FORMALS_LIST *****/

/** of [$T, ...]      */
class TypeFormalsList_AST : public AST_Tree {
protected:

    FACTORY (TypeFormalsList_AST);

    void collectDecls (Decl* enclosing) override {
        const Environ* env = enclosing->getEnviron ();
        for (auto c : *this) {
            string name = c->text ();
            if (env->findImmediate (name) != nullptr) {
                error (c, "duplicate type parameter: %s",
                       name.c_str ());
            } else {
                c->addDecl (enclosing->addTypeVarDecl (c));
            }
        }
    }

};

EXEMPLAR (TypeFormalsList_AST, TYPE_FORMALS_LIST_AST);


/***** TYPED_ID *****/

/** ID::TYPE */
class TypedId_AST : public AST_Tree {
protected:

    FACTORY (TypedId_AST);

    AST_Ptr getId () override {
        return child (0);
    }

    Decl* getDecl () override {
        return getId ()->getDecl ();
    }

    const Decl_Vect& getDecls () override {
        return getId ()->getDecls ();
    }

    Type_Ptr getType () override {
        return child (1)->asType ();
    }

    void addTargetDecls (Decl* enclosing) override {
        getId ()->addTargetDecls (enclosing);
    }

    void collectTypeVarDecls (Decl* enclosing) override {
        child (1)->collectTypeVarDecls (enclosing);
    }

    void addDecl (Decl* decl) override {
        getId ()->addDecl (decl);
    }

    void resolveTypes (Decl* context, Unifier& subst) override {
        getId ()->resolveTypes (context, subst);
        if (!getId ()->setType (child (1)->asType (), subst))
            error (this, "incompatible type assignment");
    }

    void codeGen (Code_Context& context, bool constructor) override {
        // TODO: For Param Decls, print out type then id. watch out for function ptr edge case
        // For Var Decls, print out id
        child (0)->codeGen (context, constructor);
    }

    void codeGenDecls (Code_Context& context) override {
        // TODO: For Param Decls, print out type then id. watch out for function ptr edge case
        // For Var Decls, print out id
        child (1)->codeGen (context);
        context << " ";
        child (0)->codeGenDecls (context);
    }

};

EXEMPLAR (TypedId_AST, TYPED_ID_AST);


/***** ASSIGN *****/

/** TARGET(s) = EXPR */
class Assign_AST : public Typed_Tree {
protected:

    FACTORY (Assign_AST);

    void collectDecls (Decl* enclosing) override {
        child(1)->collectDecls (enclosing);
        child(0)->addTargetDecls (enclosing);
    }

    void resolveTypes (Decl* context, Unifier& subst) override {
        int errs0 = numErrors ();
        AST::resolveTypes (context, subst);
        if (!unify (child (0)->getType (), child (1)->getType (), subst)
            && errs0 == numErrors ())
            error (this, "type mismatch in assignment");
        if (!setType (child (1)->getType (), subst) && errs0 == numErrors ())
            error (this, "type mismatch in assignment");
    }

    void codeGenDecls  (Code_Context& context) override {
    }

    void codeGen (Code_Context& context, bool constructor) override {

        /* TODO: Fix me for multiple assignment*/
        if (child (0)->typeIndex() == SUBSCRIPT_ASSIGN_AST || child (0)->typeIndex() == SLICE_ASSIGN_AST) {
            child (0)->setChild(child (0)->arity () - 1, child (1));
            child (0)->codeGen (context, constructor);
            return;
        }

        if (child (0)->typeIndex() == TARGET_LIST_AST) {
            for (int i = 0; i < child (0)->arity (); i++) {
                context << "static_cast<";
                child (0)->child (i)->getType ()-> codeGen (context, constructor);
                context << ">(env->set (\"";
                context << child (0)->child (i)->uniqueName ();
                context << "\", ";

                if (child(1)->typeIndex() == ID_AST) { // fixes to multiple assign, RHS = tuple
                    context << "get<" << std::to_string(i) << ">(";
                    child (1)->codeGen(context, constructor);
                    context << "->getValue())";
                } else {
                    child (1)->child (i)->codeGen (context, constructor);
                }
                context << "));" << endl;

            }
            return;

        }

        if (!constructor && (child (0)->typeIndex() == TYPED_ID_AST || child (0)->typeIndex() == ID_AST)) {
            context << "static_cast<";
            child (0) -> getType ()-> codeGen (context, constructor);
            context << ">(env->set (\"";
            context << child (0)-> uniqueName ();
            context << "\", ";
            child (1)->codeGen (context, constructor);
            context << "))";
            return;
        }
        child (0)->codeGen (context, constructor);
        context << " = ";
        child (1)->codeGen (context, constructor);

    }

};

EXEMPLAR (Assign_AST, ASSIGN_AST);

/***** FOR *****/

/**  for target in exprs: body [ else: body ]     */
class For_AST : public AST_Tree {
public:

    For_AST () : sequence (nullptr) {
    }

protected:

    FACTORY (For_AST);

    AST_Ptr add (AST_Ptr c) override {
        AST_Tree::add (c);
        if (arity () == 4)
            setSequence ();
        return this;
    }


    AST_Ptr add (const AST_Vect& v) override {
        AST_Tree::add (v);
        if (arity () == 4)
            setSequence ();
        return this;
    }

    void resolveTypes (Decl* context, Unifier& subst) override {
        int errs0 = numErrors ();
        AST_Ptr target = child (0);
        sequence->resolveTypes (context, subst);
        target->resolveTypes (context, subst);
        if (!unify (sequence->getType (), target->getType (), subst)
            && errs0 == numErrors ()) {
            error (this, "bad sequence type in 'for' statement");
        }

        for (auto c : trimmedAST (*this, 2))
            c->resolveTypes (context, subst);
    }

    void collectDecls (Decl* enclosing) override {
        AST_Ptr target = child (0);
        for (auto c : trimmedAST (*this, 2)) {
            c->collectDecls (enclosing);
        }
        target->addTargetDecls (enclosing);
    }

    void getOverloadings (Resolver& resolver) override {
        assert (sequence != nullptr);
        AST_Tree::getOverloadings (resolver);
        sequence->getOverloadings (resolver);
    }

    AST_Ptr resolveSimpleIds (const Environ* env) override {
        sequence->resolveSimpleIds (env);
        return AST::resolveSimpleIds (env);
    }

    void codeGen (Code_Context& context, bool constructor) override {
        AST_Ptr var = child(0);
        AST_Ptr iter = child(1);
        AST_Ptr suite = child(2);
        AST_Ptr else_clause = child(3);
        AST_Ptr temp_var = makeId ("my_temp_var", location ());
        Decl* d = makeVarDecl (temp_var, NULL, newTypeVar ());
        //iter->getType()->print(cout, 0);
        temp_var->addDecl (d);
        Unifier substr;
        temp_var->setType (var->getType (), substr);
        temp_var->replaceBindings (substr);
        //temp_var->getType()->print(cout, 0);


        context << "for (auto ";
        context << temp_var->uniqueName ();
        context << " : (";
        iter->codeGen (context, constructor);
        context << ")->getVect()";
        context << ") {" << endl;
        context.indent ();

        context << "env->set (\"" << temp_var->uniqueName () << "\", " << temp_var->uniqueName () << ");" << endl;
        NODE (ASSIGN_AST, var, temp_var)->codeGen (context, constructor);
        // TODO nicer fix
        context << ";" << endl;

        context.loop_else.push_back(context.next_ctr);
        ++context.next_ctr;
        // TODO: why was constructor true
        suite->codeGen (context);

        context.indent (-1);
        context << "}" << endl;

        // else
        else_clause->codeGen(context, constructor);
        context << "else_" << to_string(context.loop_else.back()) << ":" << endl;
        context.loop_else.pop_back();
    }

    void codeGenDecls  (Code_Context& context) override {
    }

private:

    /** Initialize sequence (assuming all children present). */
    void setSequence () {
        if (sequence == nullptr) {
            sequence = NODE (CALL_AST, makeId ("__choose__", location ()),
                             child (1));
        }
    }

    /** Artificial identifier used to define the legal sequences for
     *  for statements.  Shares parameter with child(1). */
    AST_Ptr sequence;
};

EXEMPLAR (For_AST, FOR_AST);


/***** RETURN *****/

/**  return EXPR */
class Return_AST : public AST_Tree {
protected:

    FACTORY(Return_AST);

    void resolveTypes (Decl* context, Unifier& subst) override {
        AST_Tree::resolveTypes (context, subst);
        AST_Ptr expr = child (0);
        Type_Ptr returnType = context->getType ()->returnType ();
        if (!unify (returnType, expr->getType (), subst))
            error (this, "inconsistent return type");
    }

    void codeGen (Code_Context& context, bool constructor) override {
        context << "return ";
        //not sure if this should be a loop
        for (auto c : *this) {
            c->codeGen (context, constructor);
        }
    }

};

EXEMPLAR (Return_AST, RETURN_AST);


/***** IF *****/

/** if COND: STMT else: STMT */
class If_AST : public AST_Tree {
protected:

    FACTORY(If_AST);

    void resolveTypes (Decl* context, Unifier& subst) override {
        AST_Tree::resolveTypes (context, subst);
        if (!unify (child (0)->getType (), boolDecl->asType (), subst)) {
            error (child (0), "condition must be boolean");
        }
    }

    void codeGen (Code_Context& context, bool constructor) override {
        AST_Ptr condition = child (0);
        AST_Ptr suite = child (1);
        AST_Ptr else_clause = child (2);

        context << "if (";
        Decl* truth =  mainModule->getEnviron ()->findImmediate ("truth");
        context << truth->uniqueName() << " (";
        condition->codeGen (context, constructor);
        context << ", global_env)->getValue ()) {" << endl;
        context.indent ();

        suite->codeGen (context, constructor);

        context.indent (-1);
        context << "}" << endl;

        // else
        context << "else {" << endl;
        context.indent (1);
        else_clause->codeGen (context, constructor);
        context.indent (-1);
        context << "}" << endl;
    }

    void codeGenDecls (Code_Context& context) {
        for (auto c : trimmedAST (*this, 1)) {
            c->codeGenDecls (context);
        }
    }
};

EXEMPLAR (If_AST, IF_AST);

/***** WHILE *****/

/** while COND: STMT else: STMT */
class While_AST : public AST_Tree {
protected:

    FACTORY(While_AST);

    void resolveTypes (Decl* context, Unifier& subst) override{
        AST_Tree::resolveTypes (context, subst);
        if (!unify (child (0)->getType (), boolDecl->asType (), subst)) {
            error (child (0), "condition must be boolean");
        }
    }

    void codeGenDecls (Code_Context& context) {
    }

    void codeGen (Code_Context& context, bool constructor) override {
        AST_Ptr condition = child (0);
        AST_Ptr suite = child (1);
        AST_Ptr else_clause = child (2);

        context << "while (";
        Decl* truth =  mainModule->getEnviron ()->findImmediate ("truth");
        context << truth->uniqueName() << " (";
        condition->codeGen (context, constructor);
        context << ", global_env)->getValue ()) {" << endl;
        context.indent ();

        context.loop_else.push_back(context.next_ctr);
        ++context.next_ctr;
        suite->codeGen (context, constructor);

        context.indent (-1);
        context << "}" << endl;

        // else
        else_clause->codeGen(context, constructor);
        context << "else_" << to_string(context.loop_else.back()) << ":" << endl;
        context.loop_else.pop_back();

/*        context << "else {" << endl;*/
        //context.indent ();

        //else_clause->codeGen (context, constructor);

        //context.indent (-1);
        /*context << "}" << endl;*/
    }
};

EXEMPLAR (While_AST, WHILE_AST);

class Break_AST : public AST_Tree {
protected:
    FACTORY(Break_AST);

    void codeGenDecls (Code_Context& context) {}

    void codeGen (Code_Context& context, bool constructor) override {
        context << "goto else_" << to_string(context.loop_else.back()) << ";" << endl;
    }
};

EXEMPLAR (Break_AST, BREAK_AST);

class Continue_AST : public AST_Tree {
protected:
    FACTORY(Continue_AST);

    void codeGenDecls (Code_Context& context) {}

    void codeGen (Code_Context& context, bool constructor) override {
        context << "continue;" << endl;
    }
};

EXEMPLAR (Continue_AST, CONTINUE_AST);
