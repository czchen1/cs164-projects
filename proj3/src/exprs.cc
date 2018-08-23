/* -*- mode: C++; c-file-style: "stroustrup"; indent-tabs-mode: nil; -*- */

/* exprs.cc: AST subclasses related to expressions. */

/* Authors:  YOUR NAMES HERE */

#include <iostream>
#include <iomanip>
#include <cassert>
#include <set>
#include "apyc.h"
#include "ast.h"

using namespace std;

                     /*** PRIMARY EXPRESSIONS ***/

/** Represents an identifier. */
class Id_AST : public AST_Leaf {

    void print (AST_Printer& printer) override {
        printer.os << "(id " << location () << " " << text ();
        if (getDecl () != nullptr)
            printer.os << " " << getDecl ()->getIndex ();
        printer.os << ")";
    }

    AST_Ptr getId () override {
        return this;
    }

    bool isType () override {
        Decl* me = getDecl ();
        return (me != nullptr && me->isType ());
    }

    bool hasType () override {
        return getDecl () != nullptr && getDecl ()->hasType ();
    }

    Type_Ptr computeType () override {
        return newTypeVar ();
    }

    int getNumDecls () override {
        return _me.size ();
    }

    const Decl_Vect& getDecls () override {
        return _me;
    }

    void addDecl (Decl* decl) override {
        _me.push_back (decl);
    }

    Decl* getDecl () override {
        if (_me.size () == 0)
            return nullptr;
        return _me[_index];
    }

    void setDecl (int k) override {
        if (k < 0 || k >= _me.size ())
            throw range_error ("declaration selection out of range");
        _index = k;
    }

    void removeDecl (int k) override {
        assert (k >= 0 && k < (int) _me.size ());
        _me.erase (_me.begin () + k);
    }

    void addTargetDecls (Decl* enclosing) override {
        string name = text ();
        Decl* old = enclosing->getEnviron ()->findImmediate (name);
        if (old == nullptr) {
            addDecl (enclosing->addVarDecl(this));
        } else if (! old->assignable ()) {
            error (this, "attempt to assign to non-assignable object: %s",
                   name.c_str ());
            addDecl (old);
        } else {
            addDecl (old);
        }
    }

    AST_Ptr resolveSimpleIds (const Environ* env) override {
        string name = text ();
        if (getDecl () != nullptr)
            return this;
        Decl_Vect defns;
        env->find (name, defns);
        if (defns.empty ()) {
            error (this, "undefined identifier: %s", name.c_str ());
            addDecl (makeUnknownDecl (this, false));
            return this;
        }
        if (defns.size () == 1 && defns[0]->isType ()) {
            AST_Ptr asTypeId;
            addDecl (defns[0]);
            if (defns[0]->getTypeArity () != 0) {
                error (this, "wrong number (0) of type parameters");
                asTypeId = defns[0]->asGenericType ();
            } else
                asTypeId = defns[0]->asType ();
            asTypeId->setLoc (location ());
            return asTypeId;
        }
        for (auto d : defns)
            addDecl (d);
        return this;
    }

    void getOverloadings (Resolver& resolver) override {
        resolver.addId (this);
    }

    void resolveTypes (Decl* context, Unifier& subst) override {
        Decl* decl = getDecl ();
        if (decl != nullptr && !decl->isType ()) {
            if (!unify (decl->getType (), getType (), subst))
                error (this, "no suitable declaration of %s",
                       text ().c_str ());
        }
    }

    void findUsedDecls (Decl_Set& used) override {
        if (getNumDecls () > 0) {
            used.insert (getDecl ());
        }
        getType ()->findUsedDecls (used);
    }

    void reachableDecls (Decl_Set& decls, AST_Set& visited) override {
        if (visited.count (this) == 0) {
            visited.insert (this);
            decls.insert (_me.begin (), _me.end ());
        }
    }

    AST_Ptr replaceBindings (const Unifier& subst, AST_Set& visiting) override {
        AST::replaceBindings (subst, visiting);
        for (auto d : getDecls ()) {
            d->replaceBindings (subst);
        }
        return this;
    }

    void freeTypeVarCheck (const Type_Set& allowed) override {
        if (getDecl ()->assignable ()) {
            Type_Set typeVars;
            Type_Set visited;
            getType ()->getTypeVariables (typeVars, visited);
            for (auto c : allowed)
                typeVars.erase (c);
            if (!typeVars.empty ()) {
                error (this, "type of %s has free type variables",
                       text ().c_str ());
            }
        }
    }

    void codeGenDecls (Code_Context& context) {
        string name = this->text ();
        if (name.compare ("self") == 0)
            context << "this";
        else if (name.compare ("__str__") == 0 || name.compare ("__init__") == 0  )
            context << this->text ();
        else
            context << this->text () << "_" << to_string(getDecl ()-> getIndex ());
    }

    void codeGen (Code_Context& context, bool constructor) {
        if (constructor) {
            codeGenDecls (context);
        }
        else {
            string name = this->text ();
            if (name.compare ("self") == 0)
                context << "this";
            else {
                context << "static_cast<";
                getType ()->codeGen (context);
                context << "> (" << "env->get (\"" << this->text () << "_" << to_string(getDecl ()-> getIndex ()) << "\"))";
            }
        }

    }


    FACTORY (Id_AST);

public:

    Id_AST () : _index (0) { }

private:

    /** Possible Decls that represent me. */
    Decl_Vect _me;
    /** Index in _me of the current value of getDecl (). */
    int _index;
};

EXEMPLAR (Id_AST, ID_AST);


/** Superclass of leaves that represent values having types. */
class Typed_Leaf : public AST_Leaf
{
protected:

    bool hasType () override {
        return true;
    }

};

/** Represents an integer literal. */
class Int_Literal_AST : public Typed_Leaf {
private:

    void print (AST_Printer& printer) override {
        printer.os << "(int_literal " << location () << " " << value << ")";
    }

    /** Augment setText to initialize value from the text of the lexeme,
     *  checking that the literal is in range. */
    void setText (const string& text) override {
        AST_Leaf::setText(text);
        try {
            size_t opos = text.find_first_of ("oO");
            if (opos != string::npos) {
                value = stoi (text.substr(opos+1), nullptr, 8);
            } else {
                value = stoi (text, nullptr, 0);
            }
        } catch (out_of_range excp) {
            error (location (), "integer literal out of range");
            value = 0;
        }
    }

    Type_Ptr computeType () override {
        return intDecl->asType ();
    }

    void codeGen (Code_Context& context, bool constructor) {
        context << "new Int_Obj (" << to_string (value) << ")";
    }

    long value;

    FACTORY(Int_Literal_AST);

};

EXEMPLAR(Int_Literal_AST, INT_LITERAL_AST);


/** A string literal. In Python, a string may be denoted by a number of
 *  string and raw string tokens concatentated together.  For example:
 *       message = 'He said, "Don"' "'t do that!"
 *  A String_Literal_AST contains a sequence of string tokens and,
 *  when printing an AST, concatenates them together. **/
class String_Literal_AST : public Typed_Leaf {
public:

    string denotedText () const override {
        return literalText;
    }

    void setText (const string& text) override {
        literalText.clear ();
        AST_Leaf::setText ("");
        appendText(text);
    }

    void appendText(const string& s0) override {
        if (s0.empty ()) {
            return;
        }

        string s (s0);

        if (!text ().empty ()) {
            AST_Leaf::appendText (" ");
        }
        AST_Leaf::appendText (s);

        for (size_t p = s.find ("\r\n"); p != string::npos;
             p = s.find ("\r\n", p + 2)) {
            s.replace(p, 2, "\n");
        }
        for (size_t p = s.find ('\r'); p != string::npos;
             p = s.find ('\r', p + 1)) {
            s[p] = '\n';
        }

        if (s[0] == 'r') {
            if (s.compare (0, 4, "r'''") == 0
                || s.compare (0, 4, "r\"\"\"") == 0
                || s.compare (0, 4, "R'''") == 0
                || s.compare (0, 4, "R\"\"\"") == 0) {
                literalText += s.substr (4, s.size () - 7);
            } else {
                literalText += s.substr (2, s.size () - 3);
            }
        } else {
            int v;
            size_t i;

            if (s.compare (0, 3, "'''") == 0
                || s.compare (0, 3, "\"\"\"") == 0) {
                s = s.substr (3, s.size () - 6);
            } else {
                s = s.substr (1, s.size () - 2);
            }

            i = 0;
            while (i < s.size ()) {
                i += 1;
                if (s[i-1] == '\\') {
                    i += 1;
                    switch (s[i-1]) {
                    default: literalText += '\\'; v = s[i-1]; break;
                    case '\r':
                    {
                        if (s[i] == '\n') {
                            i += 1;
                            continue;
                        } else {
                            error (location(), "stray carriage return");
                        }
                        break;
                    }
                    case '\n': continue;
                    case 'a': v = '\007'; break;
                    case 'b': v = '\b'; break;
                    case 'f': v = '\f'; break;
                    case 'n': v = '\n'; break;
                    case 'r': v = '\r'; break;
                    case 't': v = '\t'; break;
                    case 'v': v = '\v'; break;
                    case '\'': v = '\''; break;
                    case '"': case '\\': v = s[i-1]; break;
                    case '0': case '1': case '2': case '3': case '4':
                    case '5': case '6': case '7':
                    {
                        v = s[i-1] - '0';
                        for (int j = 0; j < 2; j += 1) {
                            if ('0' > s[i] || s[i] > '7')
                                break;
                            v = v*8 + (s[i] - '0');
                            i += 1;
                        }
                        break;
                    }
                    case 'x': {
                        if (i+2 > s.size () ||
                            !isxdigit (s[i]) || !isxdigit (s[i+1])) {
                            error (location (),
                                   "bad hexadecimal escape sequence");
                            break;
                        }
                        sscanf (s.c_str ()+i, "%2x", &v);
                        i += 2;
                        break;
                    }
                    }
                } else
                    v = s[i-1];
                literalText += (char) v;
            }
        }
    }

    void print (AST_Printer& printer) override {
        printer.os << "(string_literal " << location () << " \"";
        for (size_t i = 0; i < literalText.size (); i += 1) {
            int c = literalText[i] & 0377;
            if (c < '\040' || c == '\\' || c == '"' || c >= '\177') {
                printer.os << "\\" << oct << setw (3) << setfill('0') << c
                    << setfill (' ') << dec;
            } else
                printer.os << (char) c;
        }
        printer.os << "\")";
    }

    Type_Ptr computeType () override {
        return strDecl->asType ();
    }

    void codeGen (Code_Context& context, bool constructor) {
        context  << "new Str_Obj (" << "R\"(" << literalText << ")\")";
    }


private:
    FACTORY(String_Literal_AST);

    string literalText;
};

EXEMPLAR(String_Literal_AST, STRING_LITERAL_AST);


                 /* OTHER EXPRESSIONS */

/* The following are EXAMPLES (possibly incomplete) of potentially
 * useful AST classes. */

/*****   NONE, TRUE, FALSE     *****/

class None_AST : public Typed_Tree {
protected:

    FACTORY (None_AST);

    void codeGen (Code_Context& context, bool constructor) {
        context << "new Any_Obj()";
    }

};

EXEMPLAR(None_AST, NONE_AST);

class True_AST : public Typed_Tree {
protected:

    FACTORY (True_AST);

    Type_Ptr computeType () override {
        return boolDecl->asType ();
    }

    void codeGen (Code_Context& context, bool constructor) {
        context << "new Bool_Obj (true)";
    }

};

EXEMPLAR(True_AST, TRUE_AST);


class False_AST : public Typed_Tree {
protected:

    FACTORY (False_AST);

    Type_Ptr computeType () override {
        return boolDecl->asType ();
    }

    void codeGen (Code_Context& context, bool constructor) {
        context << "new Bool_Obj (false)";
    }

};

EXEMPLAR(False_AST, FALSE_AST);


/*****   CALLS    *****/

/** The supertype of "callable" things, including ordinary calls,
 *  binary operators, unary operators, subscriptions, and slices. */

class Callable : public Typed_Tree {
protected:

    /** Returns the expression representing the quantity that is
     *  called to evaluate this expression. */
    virtual AST_Ptr calledExpr () {
        return child (0);
    }

    /** Returns the number of actual parameters in this call. */
    virtual int numActuals () {
        return arity () - 1;
    }

    /** Return the Kth actual parameter in this call. */
    virtual AST_Ptr actualParam (int k) {
        return child (k + 1);
    }

    /** Set the Kth actual parameter in this call to EXPR. */
    virtual void setActual (int k, AST_Ptr expr) {
        setChild (k + 1, expr);
    }

    /** Perform type resolution on me, assuming my children have been
     *  resolved.  Arguments are as for resolveTypes. */
    virtual void resolveMyType (Decl* context, Unifier& subst) {
        Type_Ptr funcType;
        funcType = calledExpr ()->getType ();

        if (!makeFuncType (numActuals (), funcType, subst)) {
            error (this,
                   "called object is not a %d-parameter function",
                   numActuals ());
            return;
        }

        funcType = subst[funcType];
        setExprType (subst);


        for (int i = 0; i < numActuals (); i += 1) {
            if (!unify (actualParam (i)->getType (),
                        funcType->paramType (i), subst)) {
                error (this, "non-matching parameter type (parameter #%d)",
                       i);
                break;
            }
        }
    }


    /** Set my type. */
    virtual void setExprType (Unifier& subst) {
        if (!setType (subst[calledExpr ()->getType ()]->returnType (),
                      subst)) {
            error (this, "inappropriate function return type");
            return;
        }
    }

    void resolveTypes (Decl* context, Unifier& subst)  override {
        AST::resolveTypes (context, subst);
        resolveMyType (context, subst);
    }

    void codeGen (Code_Context& context, bool constructor) override {
        context << "static_cast<";
        child (0) -> getType ()->child (0)->codeGen (context, constructor);
        context << "> (";
        child(0)->codeGen (context, constructor);
        context << "->call(";
        commaSeparatedCodeGen (context, 1);
        context << ")";
        context << ")";
    }
};

/** A function call. */
class Call_AST : public Callable {
protected:

    FACTORY (Call_AST);

    AST_Ptr resolveAllocators (const Environ* env) override {
        Callable::resolveAllocators (env);
        AST_Ptr callable = calledExpr ();
        if (callable->isType ()) {
            AST_Vect initArgs;
            AST_Ptr init =
                NODE(ATTRIBUTEREF_AST, callable,
                     makeId ("__init__", location ()));
            initArgs.push_back (init);
            initArgs.push_back (NODE (NEW_AST, callable));
            for (int i = 0; i < numActuals (); i += 1)
                initArgs.push_back (actualParam (i));
            return NODE (CALL1_AST, initArgs);
        }
        return this;
    }

};

EXEMPLAR (Call_AST, CALL_AST);

/** A binary operator. */
class Binop_AST : public Callable {

    FACTORY (Binop_AST);

};

EXEMPLAR (Binop_AST, BINOP_AST);

/** A unary operator. */
class Unop_AST : public Callable {

    FACTORY (Unop_AST);

};

EXEMPLAR (Unop_AST, UNOP_AST);



/***** COMPARE *****/

static unordered_map <string, string> cmpr_ops({
        {"__gt__", ">"},
        {"__lt__", "<"},
        {"__le__", "<="},
        {"__ge__", ">="},
        {"__eq__", "=="},
        {"__ne__", "!="}
});

/** A comparison yielding bool */
class Compare_AST : public Binop_AST {
protected:

    FACTORY (Compare_AST);

    Type_Ptr computeType () override {
        return boolDecl->asType ();
    }


    void codeGen (Code_Context& context, bool constructor) override {
        set<string> compare_funcs = {"__in__", "__notin__", "__is__", "__isnot__"};
        if (compare_funcs.find(child(0)->text()) != compare_funcs.end()) {
            Callable::codeGen (context, constructor);
            return;
        }
        context << "new Bool_Obj (*(";
        child (1)->codeGen (context, constructor);
        context << ") " << cmpr_ops[child (0)->text ()] << " *(";
        child (2)->codeGen (context, constructor);
        context << "))";
    }

};

EXEMPLAR (Compare_AST, COMPARE_AST);



/***** LEFT_COMPARE *****/

/** A comparison yielding its second operand's type. */
class LeftCompare_AST : public Binop_AST {
protected:

    FACTORY (LeftCompare_AST);

    void setExprType (Unifier& subst) override {
        if (!setType (actualParam (1)->getType (), subst)) {
            error (this, "inappropriate type");
            return;
        }
    }

    Type_Ptr computeType () override {
        return actualParam (1)->getType ();
    }

    void codeGen (Code_Context& context, bool constructor) override {
//        if (child(1)->typeIndex () != LEFT_COMPARE_AST) {
//            context << "*(";
//            child (1)->codeGen (context, constructor);
//            context << ")";
//        }
//        else
//            child (1)->codeGen (context, constructor);
        context << "(*(";
        child (1)->codeGen (context, constructor);
        context << ") " << cmpr_ops[child (0)->text ()] << " ";
        context << "*(";
        child (2)->codeGen (context, constructor);
        context << "))";


/*        context << "((";*/
        //Callable::codeGen (context, constructor);
        //context << ")->getValue()? ";
        //child (2)->codeGen (context, constructor);
        /*context << " : NULL)";*/
    }

};

EXEMPLAR (LeftCompare_AST, LEFT_COMPARE_AST);

/***** CALL1 *****/

/** __init__(new T, ...)      */
class Call1_AST : public Call_AST {
protected:

    FACTORY (Call1_AST);

    Type_Ptr computeType () override {
        return actualParam (0)->getType ();
    }

    void codeGen (Code_Context& context, bool constructor) override {
        context << "new ";
        child(1)->codeGen(context);
        context << "_Obj";
        /* if (arity () > 1) {*/
            //context << "<";
            //commaSeparatedCodeGen (context, 1);
            //context << ">";
        /*}*/
        context << "(";
        commaSeparatedCodeGen (context, 2);
        context << ")";
    }

};

EXEMPLAR (Call1_AST, CALL1_AST);


/***** SUBSCRIPTION *****/

/** E1[E2] */
class Subscription_AST : public Callable {
protected:

    FACTORY (Subscription_AST);

    void addTargetDecls (Decl* enclosing) override {
    }

};

EXEMPLAR (Subscription_AST, SUBSCRIPT_AST);

class Subscript_Assign_AST : public Callable {
protected:

    FACTORY (Subscript_Assign_AST);

    void addTargetDecls (Decl* enclosing) override {
    }

};

EXEMPLAR (Subscript_Assign_AST, SUBSCRIPT_ASSIGN_AST);


/***** SLICING *****/

/** E1[E2:E3] */
class Slicing_AST : public Callable {
protected:

    FACTORY (Slicing_AST);

    void addTargetDecls (Decl* enclosing) override {
    }

};

EXEMPLAR (Slicing_AST, SLICE_AST);

class Slicing_Assign_AST : public Callable {
protected:

    FACTORY (Slicing_Assign_AST);

    void addTargetDecls (Decl* enclosing) override {
    }

};

EXEMPLAR (Slicing_Assign_AST, SLICE_ASSIGN_AST);


/***** NEW *****/

/**  new T     */
class New_AST : public Typed_Tree {
protected:

    FACTORY (New_AST);

    void resolveTypes (Decl* context, Unifier& subst) override {
        if (!setType (child (0)->asType (), subst)) {
            error (this, "inconsistent types");
        }
    }

};

EXEMPLAR (New_AST, NEW_AST);


/***** ATTRIBUTEREF *****/

/** E.ID */
class AttributeRef_AST : public Typed_Tree {
protected:

    FACTORY (AttributeRef_AST);

    AST_Ptr getId () override {
        return child (1);
    }

    const Decl_Vect& getDecls () override {
        return getId ()->getDecls ();
    }

    int getNumDecls () override {
        return getId ()->getNumDecls ();
    }

    void addTargetDecls (Decl* enclosing) override {
    }

    AST_Ptr resolveStaticSelections (const Environ* env) override {
        AST_Ptr left = child (0)->resolveStaticSelections (env);
        if (!left->isType ())
            return this;
        AST_Ptr id = getId ();
        if (id->getDecl () != nullptr)
            return id;
        string name = id->text ();
        const Environ* members = left->getDecl ()->getEnviron ();
        Decl_Vect defns;
        members->findImmediate (name, defns);
        if (defns.empty ()) {
            error (this, "no definition of %s in type", name.c_str ());
            id->addDecl (makeUnknownDecl (id, false));
        }
        for (size_t i = 0; i < defns.size (); i += 1) {
            if (!defns[i]->isMethod ())
                error (this, "np class variables in this dialect");
            id->addDecl (defns[i]);
        }
        return id;
    }

    void resolveTypes (Decl* context, Unifier& subst) override {
        int errs0 = numErrors ();
        AST_Ptr obj = child (0);
        AST_Ptr id = getId ();
        obj->resolveTypes (context, subst);
        string name = id->text ();
        Decl* attrDecl = id->getDecl ();
        if (attrDecl == nullptr)
            return;
        Decl* expectedClassDecl = attrDecl->getContainer ();
        if (expectedClassDecl != nullptr && expectedClassDecl->isType ()) {
            vector<Type_Ptr> linkedTypes;
            linkedTypes.push_back (expectedClassDecl->asBaseType ());
            linkedTypes.push_back (attrDecl->getSelectedType ());
            freshen (linkedTypes);
            if (!unify (linkedTypes[0], obj->getType (), subst)
                || (errs0 == numErrors ()
                    && (!setType (linkedTypes[1], subst)
                        || !unify (getType (), id->getType (), subst)))) {
                error (this,
                       "attribute %s has no definition that fits the context.",
                       name.c_str ());
            }
        }
    }

    AST_Ptr resolveSimpleIds (const Environ* env) override {
        setChild (0, child (0)->resolveSimpleIds (env));
        AST_Ptr id = getId ();
        string name = id->text ();
        const Decl_Vect& decls = outerEnviron->get_members ();
        for (auto classd : decls)
            classd->addAttributeDecls (id);
        if (id->getNumDecls () == 0) {
            error (this, "no definitions of the %s attribute", name.c_str ());
            id->addDecl (makeUnknownDecl (id, false));
        }
        return this;
    }

    void codeGen (Code_Context& context, bool constructor) override {
        child (0)->codeGen (context, constructor);
        context << "->";
        child (1)->codeGenDecls (context);
    }

};

EXEMPLAR (AttributeRef_AST, ATTRIBUTEREF_AST);


/***** TUPLE *****/

/** (E1, ...)  */
class Tuple_AST : public Typed_Tree {
protected:

    FACTORY (Tuple_AST);

    void resolveTypes (Decl* context, Unifier& subst) override {
        Typed_Tree::resolveTypes (context, subst);
        if (arity () > 3)
            error (this, "tuple has too many elements (max 3)");
        Type_Vect componentTypes;
        for (auto c : *this)
            componentTypes.push_back (c->getType ());
        Type_Ptr tupleType =
            tupleDecl[arity ()]->asType (componentTypes);
        if (!setType (tupleType, subst))
            error (this, "type mismatch on tuple");
    }

    void codeGen (Code_Context& context, bool constructor) override {
        context << "new Tuple" << to_string(arity()) << "_Obj";
        if (arity() != 0) {
            context << "<";
            getType() -> commaSeparatedCodeGen(context, 1);
            context << ">";
        }
        context << "(";
        commaSeparatedCodeGen (context, 0);
        context << " )";
    }

};

EXEMPLAR (Tuple_AST, TUPLE_AST);

/***** TARGET_LIST *****/

/** (TARGET, ...) on left-hand side of assignment, for. */
class TargetList_AST : public Typed_Tree {
protected:

    FACTORY (TargetList_AST);

    void addTargetDecls (Decl* enclosing) override {
        for (auto c :  *this) {
            c->addTargetDecls (enclosing);
        }
    }

    void resolveTypes (Decl* context, Unifier& subst) override {
        if (arity () > 3)
            error (this, "target list has too many elements (max 3)");
        Type_Vect componentTypes;
        for (int i = 0; i < arity (); i += 1)
            componentTypes.push_back (newTypeVar ());
        Type_Ptr tupleType =
            tupleDecl[arity ()]->asType (componentTypes);
        if (!setType (tupleType, subst))
            error (this, "type mismatch on assignment");
        for (int i = 0; i < arity (); i += 1) {
            child (i)->resolveTypes (context, subst);
            if (!unify(child (i)->getType (), componentTypes[i], subst))
                error (child (i), "type mismatch on assignment");
        }
    }

};

EXEMPLAR (TargetList_AST, TARGET_LIST_AST);


/***** LIST_DISPLAY *****/

/** [E1, ...]  */
class ListDisplay_AST : public Typed_Tree {
protected:

    FACTORY (ListDisplay_AST);

    void resolveTypes (Decl* context, Unifier& subst) override {
        Typed_Tree::resolveTypes (context, subst);
        Type_Ptr componentType =
            arity () == 0 ? newTypeVar () : child (0)->getType ();
        for (int i = 1; i < arity (); i += 1)
            if (!unify (componentType, child (i)->getType (), subst)) {
                error (child (i), "type mismatch in list display");
                break;
            }
        Type_Ptr listType =
            listDecl->asType (AST_Vect (1, componentType));
        if (!setType (listType, subst))
            error (this, "type mismatch on list display");
    }

    void codeGen (Code_Context& context, bool constructor) {
        context << "new List_Obj <";

        getType()->child (1)->codeGen (context, constructor);

        context << "> (vector<";
        getType()->child (1)->codeGen (context, constructor);
        context << ">{";
        commaSeparatedCodeGen (context, 0);
        context << "})";

    }

};

EXEMPLAR (ListDisplay_AST, LIST_DISPLAY_AST);


/***** DICT_DISPLAY *****/

/** { a: b, ... }  */
class DictDisplay_AST : public Typed_Tree {
protected:

    FACTORY (DictDisplay_AST);

    void resolveTypes (Decl* context, Unifier& subst) override {
        Typed_Tree::resolveTypes (context, subst);
        Type_Ptr keyType, valType;
        if (arity () == 0) {
            keyType = newTypeVar ();
            valType = newTypeVar ();
        } else  {
            keyType = child (0)->child (0)->getType ();
            valType = child (0)->child (1)->getType ();
        }
        for (int i = 1; i < arity (); i += 1) {
            if (!unify (keyType, child (i)->child (0)->getType (), subst)) {
                error (child (i), "key type mismatch in dict display");
                break;
            }
            if (!unify (valType, child (i)->child (1)->getType (), subst)) {
                error (child (i), "value type mismatch in dict display");
                break;
            }
        }
        Type_Ptr dictType = dictDecl->asType (AST_Vect ({ keyType, valType }));
        if (!setType (dictType, subst))
            error (this, "type mismatch on dict display");
    }

    void codeGen (Code_Context& context, bool constructor) {
        context << "new Dict_Obj_";
        getType ()->child (1)->codeGen (context, constructor);
        context << "<";

        getType ()->child (2)->codeGen (context, constructor);
        context << "> (vector<pair<";
        getType ()->child (1)->codeGen (context, constructor);
        context << ", ";
        getType()->child (2)->codeGen(context, constructor);
        context << ">>{";

        if (arity() > 0) {
            context << "{ ";
            child (0)->codeGen (context, constructor);
            context << " }";
        }
        if (arity() > 1)
        {
            for (auto c: trimmedAST (*this, 1, -1)) {
                context << ", ";
                context << "{";
                c->codeGen (context, constructor);
                context << "}";
            }
        }
        context << "})";
    }

};

EXEMPLAR (DictDisplay_AST, DICT_DISPLAY_AST);


/***** NEXT_VALUE *****/

/** The dummy source for assignments in target lists. */
class NextValue_AST : public Typed_Tree {
protected:

    FACTORY (NextValue_AST);

};

EXEMPLAR (NextValue_AST, NEXT_VALUE_AST);


/** A class of expression in which its subexpressions must agree as to
 *  type and its value's type is that of its subexpressios. */
class BalancedExpr : public Typed_Tree {
protected:

    FACTORY (BalancedExpr);

    void resolveTypes (Decl* context, Unifier& subst) override {
        int k = arity ();
        Typed_Tree::resolveTypes (context, subst);
        if (!unify (child (k - 2)->getType (), child (k - 1)->getType (), subst))
            error (this, "types of alternatives do not match");
        if (!setType (child (k - 1)->getType (), subst))
            error (this, "inconsistent types");
    }

};

/***** IF_EXPR *****/

/**  E1 if Cond else E2  */
class IfExpr_AST : public BalancedExpr {
protected:

    void resolveTypes (Decl* context, Unifier& subst) override {
        BalancedExpr::resolveTypes (context, subst);
        if (!unify (child (0)->getType (), boolDecl->asType (), subst)) {
            error (child (0), "condition must be boolean");
        }
    }

    void codeGen(Code_Context& context, bool constructor) {
        context << "((";
        Decl* truth =  mainModule->getEnviron ()->findImmediate ("truth");
        context << truth->uniqueName() << " (";
        child(0)->codeGen (context, constructor);
        context << ", global_env)->getValue ())"; 
        context << " ? ";
        child(1)->codeGen(context, constructor);
        context << " : ";
        child(2)->codeGen(context, constructor);
        context << ")";

    }

    FACTORY (IfExpr_AST);
};


EXEMPLAR (IfExpr_AST, IF_EXPR_AST);

/***** AND *****/

/** E1 and E2 */
class And_AST : public BalancedExpr {
protected:

    void codeGen(Code_Context& context, bool constructor){
        context << "(__truth__(";
        child(0)->codeGen (context, constructor);
        context << ")->getValue() ? ";
        child(1)->codeGen (context, constructor);
        context << ": ";
        child(0)->codeGen (context, constructor);
        context << ")";
    }


    FACTORY (And_AST);

};

EXEMPLAR (And_AST, AND_AST);

/***** OR *****/

/** E1 or E2 */
class Or_AST : public BalancedExpr {
protected:


    void codeGen(Code_Context& context, bool constructor){
        context << "(__truth__(";
        child(0)->codeGen (context, constructor);
        context << ")->getValue() ? ";
        child(0)->codeGen (context, constructor);
        context << ": ";
        child(1)->codeGen (context, constructor);
        context << ")";
    }

    FACTORY (Or_AST);

};

EXEMPLAR (Or_AST, OR_AST);

class Pair_AST : public AST_Tree {
    void codeGen (Code_Context& context, bool constructor) override {
        child (0)->codeGen (context, constructor);
        context << ", ";
        child (1)->codeGen (context, constructor);
    }

protected:
    FACTORY (Pair_AST);
};

EXEMPLAR (Pair_AST, PAIR_AST);
