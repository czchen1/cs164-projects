/* -*- mode: C++; c-file-style: "stroustrup"; indent-tabs-mode: nil; -*- */

/* exprs.cc: AST subclasses related to expressions. */

/* Authors:  YOUR NAMES HERE */

#include <iostream>
#include <iomanip>
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

    Type_Ptr computeType () override {
        Decl* d = getDecl();
        if (d != NULL) {
            return d->getType();
        }
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

    void removeDecl (int k) override {
        assert (k >= 0 && k < (int) _me.size ());
        _me.erase (_me.begin () + k);
    }

    AST_Ptr resolveSimpleIds (const Environ* env) {
        Decl_Vect defns;
        // Id node already has a decl attached to it (eg: it is a declaration)
        if (this->getDecl() != NULL)
            return this;
        // Look through the immediate environ
        Decl* d = env->findImmediate(this->text());
        if (d != NULL) {
            this->addDecl(d);
            return (d->isType()) ? NODE (TYPE_AST, this) : this;
        }
        // Looks through enclosing environ, ignoring class methods/variables
        env->find (this->text(), defns);
        for (Decl* c : defns) {
            if (c->isInstance() || c->isMethod()) {
                continue;
            }
            else {
                this->addDecl(c);
                return (c->isType()) ? NODE (TYPE_AST, this) : this;
            }
        }
        error(location(), "identifier not declared: %s", this->text().c_str());
        return this;
    }

/*    void*/
    //resolveTypesOuter (Decl* context) override {
        //Unifier bind;
        //Decl *corresponding = getDecl();
        //unify(corresponding->getType(), getType(), bind);
        //corresponding->replaceBindings(bind);
    //}
    //void
    //resolveTypesInner (Decl* context, Unifier& bind) override {
        //setType(getType(), bind);
    /*}*/


    void collectDecls (Decl* enclosing) override {
        if (getDecl() != NULL)
            return;
        // Adds d to enclosing decl. If decl already exists in local space, returns that
        Decl* d = enclosing->addVarDecl(this);
        if (d == nullptr) error(location(), "Var decl not created");
        this->addDecl(d); // Adds decl to _me in Id_AST node.
    }

    virtual AST_Ptr replaceBindings (const Unifier& subst) {
        getDecl()->replaceBindings(subst);
        return this;
    }


    virtual AST_Ptr replaceBindings (const Unifier& subst, AST_Set& visiting){
        getDecl()->replaceBindings(subst);
        return this;
    }



    FACTORY (Id_AST);

private:

    Decl_Vect _me;
};

EXEMPLAR (Id_AST, ID_AST);




class Typed_Id_AST : public AST_Tree {

    FACTORY (Typed_Id_AST);

    void collectDecls (Decl* enclosing) override {
        /* Represents id::type. Do not need to create new decl for type variable. */
        AST_Ptr id = this->child(0);
        AST_Ptr type = this->child(1);
        Decl* d = enclosing->addVarDecl(id, type);
        if (d == nullptr) error(location(), "Var decl not created");
        id->addDecl(d); // Adds decl to _me in Id_AST node.

    }


/*    void*/
    //resolveTypesInner (Decl* context, Unifier& bind) override {
        //for(iterator c = begin(); c != end(); c++){
            //(*c)->resolveTypesInner(context, bind);
        //}
        //for(iterator c = begin(); c != end(); c++){
            //(*c)->resolveDeclTypes(bind);
        //}
        //unify(child(0)->getType(), child(1)->asType(), bind);
        //setType(child(0)->getType(), bind);
        //replaceBindings(bind);
/*}*/

    Type_Ptr computeType () override {
        return this->child(1)->asType();

    }

};
EXEMPLAR (Typed_Id_AST, TYPED_ID_AST);

class List_AST : public AST_Tree {
protected:
    FACTORY (List_AST);

    void
    resolveTypesOuter (Decl* context) override {
        Unifier bind;
        int i = 0;
        for(iterator c = begin(); c != end(); c++){
                (*c)->resolveTypesInner(context, bind);
        }
        Type_Ptr m;
        if (arity() == 0)
        {
            m = newTypeVar();
        }
        else
        {
            m = child(0)->getType();
        }
        while(i < arity()){
            if (!unify(m, child(i)->getType(), bind))
            {
                error (child(i), "type mismatch");
                break;
            }
            i+=1;
        }
        AST_Vect params;
        params.push_back(m);
        Type_Ptr listType = listDecl->asType (1, params);
        if (!setType (listType, bind)) {
             error (this, "type mismatch");
         }
        replaceBindings(bind);
        for(iterator c = begin(); c != end(); c++){
            (*c)->resolveDeclTypes(bind);
        }
        resolveDeclTypes(bind);
    }

    void
    resolveTypesInner (Decl* context, Unifier &bind) override {
        int i = 0;
        for(iterator c = begin(); c != end(); c++){
                (*c)->resolveTypesInner(context, bind);
        }
        Type_Ptr m;
        if (arity() == 0)
        {
            m = newTypeVar();
        }
        else
        {
            m = child(0)->getType();
        }
        while(i < arity()){
            if (!unify(m, child(i)->getType(), bind))
            {
                error (child(i), "type mismatch");
                break;
            }
            i+=1;
        }
        AST_Vect params;
        params.push_back(m);
        Type_Ptr listType = listDecl->asType (1, params);
        if (!setType (listType, bind)) {
             error (this, "type mismatch");
         }
        replaceBindings(bind);
        for(iterator c = begin(); c != end(); c++){
            (*c)->resolveDeclTypes(bind);
        }
        resolveDeclTypes(bind);
    }

};
EXEMPLAR (List_AST, LIST_DISPLAY_AST);

class Dict_AST : public AST_Tree {
protected:
    FACTORY(Dict_AST);
    void
    resolveTypesOuter (Decl* context) override {
        Unifier bind;
        for(iterator c = begin(); c != end(); c++){
            (*c)->child(0)->resolveTypesInner(context, bind);
            (*c)->child(1)->resolveTypesInner(context, bind);
        }
        Type_Ptr keyType, valType;
        if (arity () == 0) {
            keyType = newTypeVar();
            valType = newTypeVar();
        } else  {
            keyType = child (0)->child(0)->getType ();
            valType = child (0)->child(1)->getType ();
        }
        for (int i = 1; i < arity(); i += 1) {
            if (!unify (keyType, child(i)->child(0)->getType (), bind)) {
                error (child(i), "key type mismatch");
                break;
            }
            if (!unify (valType, child(i)->child(1)->getType (), bind)) {
                error (child(i), "value type mismatch");
                break;
            }
        }
        AST_Vect params;
        params.push_back(keyType);
        params.push_back(valType);
        Type_Ptr dictType = dictDecl->asType (2, params);
        if (!setType (dictType, bind)) {
             error (this, "type mismatch");
         }
        replaceBindings(bind);
        for(iterator c = begin(); c != end(); c++){
            (*c)->resolveDeclTypes(bind);
        }
        resolveDeclTypes(bind);
    }

    void
    resolveTypesInner (Decl* context, Unifier &bind) override {
        for(iterator c = begin(); c != end(); c++){
            (*c)->child(0)->resolveTypesInner(context, bind);
            (*c)->child(1)->resolveTypesInner(context, bind);
        }
        Type_Ptr keyType, valType;
        if (arity () == 0) {
            keyType = newTypeVar();
            valType = newTypeVar();
        } else  {
            keyType = child (0)->child(0)->getType ();
            valType = child (0)->child(1)->getType ();
        }
        for (int i = 1; i < arity(); i += 1) {
            if (!unify (keyType, child(i)->child(0)->getType (), bind)) {
                error (child(i), "key type mismatch");
                break;
            }
            if (!unify (valType, child(i)->child(1)->getType (), bind)) {
                error (child(i), "value type mismatch");
                break;
            }
        }
        AST_Vect params;
        params.push_back(keyType);
        params.push_back(valType);
        Type_Ptr dictType = dictDecl->asType (2, params);
        if (!setType (dictType, bind)) {
             error (this, "type mismatch");
         }
        replaceBindings(bind);
        for(iterator c = begin(); c != end(); c++){
            (*c)->resolveDeclTypes(bind);
        }
        resolveDeclTypes(bind);
    }
};
EXEMPLAR(Dict_AST, DICT_DISPLAY_AST);

class And_AST : public AST_Tree {
protected:
    FACTORY(And_AST);
    void
    resolveTypesOuter (Decl* context) override {
        Unifier bind;
        AST_Ptr left = child(0);
        AST_Ptr right = child(1);
        left->resolveTypesInner(context, bind);
        right->resolveTypesInner(context, bind);
        for(iterator c = begin(); c != end(); c++){
            (*c)->resolveDeclTypes(bind);
        }
        if(!(left->getType()->isTypeVariable())){
            //TODO: Check right side has same type as left
        }
        unify(left->getType(), right->getType(), bind);
        left->replaceBindings(bind);
        right->replaceBindings(bind);
        setType(left->getType(), bind);
        replaceBindings(bind);
        for(iterator c = begin(); c != end(); c++){
            (*c)->resolveDeclTypes(bind);
        }
    }
    void
    resolveTypesInner (Decl* context, Unifier& bind) override {
        AST_Ptr left = child(0);
        AST_Ptr right = child(1);
        left->resolveTypesInner(context, bind);
        right->resolveTypesInner(context, bind);
        for(iterator c = begin(); c != end(); c++){
            (*c)->resolveDeclTypes(bind);
        }
        if(!(left->getType()->isTypeVariable())){
            //TODO: Check right side has same type as left
        }
        unify(left->getType(), right->getType(), bind);
        left->replaceBindings(bind);
        right->replaceBindings(bind);
        setType(left->getType(), bind);
        replaceBindings(bind);
    }
};
EXEMPLAR(And_AST, AND_AST);

class Or_AST : public AST_Tree {
protected:
    FACTORY(Or_AST);
    void
    resolveTypesOuter (Decl* context) override {
        Unifier bind;
        AST_Ptr left = child(0);
        AST_Ptr right = child(1);
        left->resolveTypesInner(context, bind);
        right->resolveTypesInner(context, bind);
        for(iterator c = begin(); c != end(); c++){
            (*c)->resolveDeclTypes(bind);
        }
        if(!(left->getType()->isTypeVariable())){
            //TODO: Check right side has same type as left
        }
        unify(left->getType(), right->getType(), bind);
        left->replaceBindings(bind);
        right->replaceBindings(bind);
        setType(left->getType(), bind);
        replaceBindings(bind);
        for(iterator c = begin(); c != end(); c++){
            (*c)->resolveDeclTypes(bind);
        }
    }
    void
    resolveTypesInner (Decl* context, Unifier& bind) override {
        AST_Ptr left = child(0);
        AST_Ptr right = child(1);
        left->resolveTypesInner(context, bind);
        right->resolveTypesInner(context, bind);
        for(iterator c = begin(); c != end(); c++){
            (*c)->resolveDeclTypes(bind);
        }
        if(!(left->getType()->isTypeVariable())){
            //TODO: Check right side has same type as left
        }
        unify(left->getType(), right->getType(), bind);
        left->replaceBindings(bind);
        right->replaceBindings(bind);
        setType(left->getType(), bind);
        replaceBindings(bind);
    }
};
EXEMPLAR(Or_AST, OR_AST);

//TO DO: This; dont get the built in thing yet
class Tuple_AST : public AST_Tree {

    FACTORY (Tuple_AST);

    Type_Ptr computeType () override {
    /* Returns type -- a tuple with type of each elements*/
        int arity = children().size();
        if (arity > 3)
            error(location(), "tuples only allowed of length < 4");
        AST_Vect params;
        for (auto c : *this) {
            params.push_back(c->getType());
        }
        return tupleDecl[arity]->asType(arity, params);
    }

    void
    resolveTypesOuter (Decl* context) override {
    }

    void
    resolveTypesInner (Decl* context, Unifier& bind) override {
    }

};
EXEMPLAR (Tuple_AST, TUPLE_AST);

class Formals_AST : public AST_Tree {

    FACTORY (Formals_AST);

    void
    resolveTypesInner (Decl* context, Unifier& bind) override {
        for(iterator c = begin(); c != end(); c++){
            (*c)->resolveTypesInner(context, bind);
        }
        for(iterator c = begin(); c != end(); c++){
            (*c)->resolveDeclTypes(bind);
        }
        int i = 0;
        for(iterator c = begin(); c != end(); c++){
            //unify((*c)->child(0)->getDecl()->getType(), t, bind);  // bind the typevardecl to the corresponding paramdecl  just dont get point of typevardecl's
            unify(context->getType()->paramType(i), (*c)->getType(), bind);  // bind the type of each paramdecl to the function type arguments
            //context->getType()->paramType(i)->addDecl(child(0)->getDecl());  attempting to bind types to paramdecls to create a link to them
            i += 1;
        }
        replaceBindings(bind);
        resolveDeclTypes(bind);
        context->replaceBindings(bind);

    }
};
EXEMPLAR (Formals_AST, FORMALS_LIST_AST);


class Target_List_AST : public AST_Tree {

    FACTORY (Target_List_AST);

    Type_Ptr computeType () override {
    /* Returns type -- a tuple with type of each elements*/
        int arity = children().size();
        AST_Vect params;
        for (auto c : *this) {
            params.push_back(c->getType());
        }
        return tupleDecl[arity]->asType(arity, params);
    }

    void resolveTypesInner (Decl* context, Unifier& bind) override {
        for (auto c : *this) {
            c->resolveTypesInner(context, bind);
        }
    }


};
EXEMPLAR (Target_List_AST, TARGET_LIST_AST);

/** Represents an integer literal. */
class Int_Literal_AST : public AST_Leaf {
private:

    void print (AST_Printer& printer) override {
        printer.os << "(int_literal " << location () << " " << value << ")";
    }

    /** Augment setText to initialize value from the text of the lexeme,
     *  checking that the literal is in range. */
    void setText (const string& text) override {
        AST_Leaf::setText(text);
        if (text[1] == 'o' || text[1] == 'O') {
            value = stol (text.substr (2), NULL, 8);
        } else {
            value = stol (text, NULL, 0);
        }
    }

    Type_Ptr computeType () override {
        return intDecl->asType ();
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
class String_Literal_AST : public AST_Leaf {
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

    Type_Ptr computeType () override {
        return newTypeVar();
    }

};

EXEMPLAR(None_AST, NONE_AST);

class True_AST : public Typed_Tree {
protected:

    FACTORY (True_AST);

    Type_Ptr computeType () override {
        return boolDecl->asType ();
    }

};

EXEMPLAR(True_AST, TRUE_AST);


class False_AST : public Typed_Tree {
protected:

    FACTORY (False_AST);

    Type_Ptr computeType () override {
        return boolDecl->asType ();
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

    // PUT COMMON CODE DEALING WITH TYPE-CHECKING or SCOPE RULES HERE.
    Type_Ptr computeType () override {
        if (_functype == NULL)
            return NULL;
        return _functype->child(0)->asType();
    }
   

    void setFuncType (Type_Ptr functype) {
        _functype = functype;
    }
    

private:
    Type_Ptr _functype;

};

/** A function call. */
class Call_AST : public Callable {
protected:

    FACTORY (Call_AST);

    void collectDecls (Decl* enclosing) override {
    }
    
    Type_Ptr matchParams (Decl_Vect& defns, int num_args, Unifier& bind) {
        /* Looks through defns to find matching function call
         * If one is found, add it as a decl to the function id node
         * If two functions with same signature are found, throw error*/
        Type_Ptr func_type;
        Type_Ptr matched_func_type;
        bool foundMatchingFunction = false; 
        for (Decl* c : defns) {
            // If not overloadable, it's not a function
            if (!c->isOverloadable() && !c->getType()->isFunctionType ()) {
                error(location(), "Non-function or constructor variable called"); 
                return NULL;
            }
            func_type = freshen(c->getType());
            //c->getType()->print(cout,0);
            //cout <<endl<< "num args " << num_args << " num params" << c->getType()->numParams();
            //func_type->print(cout,0);
            // Number of params should match number of arguments
            if (num_args == (c->getType()->numParams())) {
                //cout << endl << "same number params" << endl;
                bool paramsMatch = true;
                // loop through params checking if type is correct
                for (int j = 1; j < num_args+1; j++) {
                    //cout << "param" << endl;
                    Type_Ptr param = func_type->child(j)->asType();
                    //param -> print (cout, 0);
                    //cout << "argument" << endl;
                    Type_Ptr argument = child(j)->getType();  
                    //argument -> print (cout, 0);
                    if (!unify(param, argument, bind)) {
                        paramsMatch = false;
                        break;
                    }
                }
                if (paramsMatch) {
                    if (foundMatchingFunction) {
                        error(location(), "ambiguous function definition");
                        return NULL;
                    }
                    child(0)->addDecl(c);
                    matched_func_type = func_type;
                    foundMatchingFunction = true;
                }
            }
/*            else */
                /*cout << endl << "diff number params" << endl;*/

        }
        if (!foundMatchingFunction) {
            error(location(), "No matching function definition");
            return NULL;
        }
        return matched_func_type;
    }

    void resolveTypesInner (Decl* context, Unifier& bind) override {
        /** must be function or expr resolving to function at this point*/
        // Start by assuming it's an id. Remove dummy decl.
        AST_Ptr id = child(0);
        if (id->getNumDecls() > 0)
            id->removeDecl(0);

        // Resolve params. Now getType on a param should work.
        int num_args = arity() - 1;
        for (size_t i = 1; i < num_args + 1; i++) {
            // Inner, because free variables are allowed
            child(i)->resolveTypesInner(context, bind);
        }

        // Look through the immediate environ
        Decl_Vect defns;
        context->getEnviron()->find(id->text(), defns);
        if (defns.size() == 0) {
            error(location(), "identifier not declared: %s", id->text().c_str());
            return;
        }
        Type_Ptr func_type = matchParams (defns, num_args, bind); 
        if (func_type==NULL)
            return;
        func_type->replaceBindings(bind);
        setFuncType(func_type);
        //func_type->print(cout, 0);
    }

    AST_Ptr resolveAllocators (const Environ* env) override {

        AST_Vect children = this->children();
        AST_Ptr lhs = this->child(0);
        if (lhs->isType()) {
            AST_Ptr init = makeId("__init__", location());
            AST_Ptr new_node = NODE(NEW_AST, lhs);
            children.erase(children.begin());
            children.insert(children.begin(), new_node);
            children.insert(children.begin(), init);
            return NODE(CALL1_AST, children);
        } 
        return this;
    }
    
    void resolveTypesOuter (Decl* context) override {
        Unifier bind;
        resolveTypesInner(context, bind);

    }

 
    /*
    void
    resolveTypesInner (Decl* context, Unifier& bind) override {
        for(iterator c = begin(); c != end(); c++){
            (*c)->resolveTypesInner(context, bind);
            (*c)->resolveDeclTypes(bind);
        }
        Type_Ptr funcType = child(0)->getType();
        Decl_Vect* params = child(0)->getDecl()->getParams();
        int i = 0;
        while(i < funcType->numParams()){
            if(!(unify(funcType->paramType(i), child(i + 1)->getType(), bind))){
                error(location(), "Invalid types");
            }
            i += 1;
        }
        i = 0;
        for(std::vector<Decl*>::iterator k = params->begin(); k != params->end(); k++){
            unify(funcType->paramType(i), (*k)->getType(), bind);
            (*k)->replaceBindings(bind);
            i += 1;
        }
        setType(funcType, bind);
        context->replaceBindings(bind);
        replaceBindings(bind);
        resolveDeclTypes(bind);

    } */ 
};
EXEMPLAR (Call_AST, CALL_AST);

class Call1_AST : public Callable {
protected:

    FACTORY(Call1_AST);

    void
    resolveTypesInner (Decl* context, Unifier& bind) override {
        AST_Ptr init_node = child(0);
        for (int i = 2; i < children().size(); i++){
            child(i)->resolveTypesInner(context, bind);
            child(i)->replaceBindings(bind);
            Type_Ptr t = child(i)->getType();
        }
        
        //setType(, bind);
    }

    AST_Ptr resolveStaticSelections(const Environ* env) {
        AST_Ptr id = child(0);
        AST_Ptr type = child(1)->child(0)->child(0);
        if (id->getDecl() != NULL)
            return id;
        Decl_Vect defns;
        const Environ* members = type->getDecl ()->getEnviron ();
        members->findImmediate(id->text(), defns);
        if (defns.empty()) {
            error(this, "no __init__ defined");
        }
        for (Decl* c : defns) {
            id->addDecl(c);
        }
        return this;
    }
};
EXEMPLAR(Call1_AST, CALL1_AST);



class New_AST : public AST_Tree {
protected:

    FACTORY(New_AST);

    void
    resolveTypesInner (Decl* context, Unifier& bind) override {
        setType(child(0)->asType(), bind);
        replaceBindings(bind);

    }
};
EXEMPLAR(New_AST, NEW_AST);

/** A binary operator. */
class Binop_AST : public Callable {

    FACTORY (Binop_AST);
    // FIXME
};

EXEMPLAR (Binop_AST, BINOP_AST);

/** A unary operator. */
class Unop_AST : public Callable {

    FACTORY (Unop_AST);

    // FIXME
};

EXEMPLAR (Unop_AST, UNOP_AST);

// FIXME: There are others as well.

class AttributeRef_AST : public Typed_Tree {
    protected:

    FACTORY(AttributeRef_AST);

    void collectDecls (Decl* enclosing) override {
    }


    AST_Ptr resolveSimpleIds(const Environ* env) {
        setChild(0, child(0)->resolveSimpleIds(env));
        AST_Ptr id = child(1);
        Decl_Vect defns;
        defns = outerEnviron->get_members();
        for (Decl* c : defns) {
            if (c->isType()) {
                Decl_Vect type_defns;
                c->getEnviron()->findImmediate(id->text(), type_defns);
                for (Decl* t : type_defns) {
                    id->addDecl(t);
                }
            }
        }
        return this;
    }


    AST_Ptr resolveStaticSelections(const Environ* env) {
        AST_Ptr left = child(0);
        if (!left->isType ()) {
            //error(this, "not referencing a class");
	        return this;
    	}

        AST_Ptr id = child(1);
        if (id->getDecl() != NULL)
            return id;
        Decl_Vect defns;
        const Environ* members = left->getDecl ()->getEnviron ();
        members->findImmediate(id->text(), defns);
        if (defns.empty()) {
            error(this, "no definition in type");
        }
        for (Decl* c : defns) {
	        if (!c->isMethod()) {
		        error(this, "can't reference instance variables");
	        }
	        id->addDecl(c);
	    }
        return id;
    }
    void
    resolveTypesInner (Decl* context, Unifier& bind) override {
        if (!setType(child(1)->getDecl()->getType(), bind)) {
            error (location(), "attributeref has the wrong id type!");
        }
    }
};

EXEMPLAR (AttributeRef_AST, ATTRIBUTEREF_AST);

class Subscript_AST : public Callable {
    FACTORY (Subscript_AST);
};
EXEMPLAR (Subscript_AST, SUBSCRIPT_AST);

class Slice_AST : public Callable {
    FACTORY (Slice_AST);
};
EXEMPLAR (Slice_AST, SLICE_AST);

class Subscript_Assign_AST : public Callable {
    FACTORY (Subscript_Assign_AST);
};
EXEMPLAR (Subscript_Assign_AST, SUBSCRIPT_ASSIGN_AST);

class Slice_Assign_AST : public Callable {
    FACTORY (Slice_Assign_AST);
};
EXEMPLAR (Slice_Assign_AST, SLICE_ASSIGN_AST);

class Compare_AST : public Callable {
    FACTORY (Compare_AST);
};
EXEMPLAR (Compare_AST, COMPARE_AST);

class Left_Compare_AST : public Callable {
    FACTORY (Left_Compare_AST);
};
EXEMPLAR (Left_Compare_AST, LEFT_COMPARE_AST);

class If_Expr_AST : public AST_Tree {
    FACTORY (If_Expr_AST);

    void
    resolveTypesOuter (Decl* context) override {
        Unifier bind;
        resolveTypesInner (context, bind);
    }

    void
    resolveTypesInner (Decl* context, Unifier& bind) override {
        child(0)->resolveTypesInner(context, bind);
        child(1)->resolveTypesInner(context, bind);
        child(2)->resolveTypesInner(context, bind);
        if (!child(0)->setType(boolDecl->asType(), bind)) {
            error (location(), "if_expr condition is not a bool");
        }

        Type_Ptr expr_type = child(1)->getType();
        if (!child(2)->setType(expr_type, bind)) {
            error (location(), "if_expr child statements have differing types");
        }
        if (!setType(expr_type, bind)) {
            error (location(), "if_expr entire expression has the wrong type");
        }
        // TODO: call replaceBindings? I'm honestly too tired to figure out if it needs to be called.
   }
};
EXEMPLAR (If_Expr_AST, IF_EXPR_AST);
