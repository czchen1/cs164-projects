/* -*- mode: C++; c-file-style: "stroustrup"; indent-tabs-mode: nil; -*- */

/* ast.cc: Basic AST subclasses. */

/* Authors  P. N. Hilfinger */

#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <vector>
#include <cstdarg>
#include "apyc.h"
#include "ast.h"

using namespace std;

/** A table of external representations of node types for text
 *  representation indexed by their AST_Index. */

static const char* astNames[] = {

#define AST_INFO(SYM, NAME) NAME,
#include "ast-types.i"
#undef AST_INFO

}; 

/** A table of exemplars of node types, used by factory methods. A
 *  null entry indicates that a generic tree or leaf node is to
 *  to be used for the type. */
static AST_Ptr exemplars[LAST_AST];

/* Definitions of methods in base class AST. */

AST::AST () : loc (0L), index (EMPTY_AST), _type (nullptr)
{
}

Location
AST::location () const
{
    return loc;
}

void
AST::setLoc (Location loc)
{
    this->loc = loc;
}

AST_Index
AST::typeIndex () const
{
    return index;
}

void
AST::setTypeIndex (AST_Index syn)
{
    if ((unsigned int) syn >= LAST_AST) {
        throw domain_error ("bad node typeIndex value");
    }
    index = syn;
}

const char*
AST::astTypeName () const
{
    return astNames[index];
}

string
AST::text () const
{
    throw logic_error ("node has no text");
}

void
AST::setText (const string& text)
{
    throw logic_error ("node has no text");
}

void
AST::appendText (const string& text)
{
    throw logic_error ("node has no text");
}

string
AST::denotedText () const
{
    return text ();
}

int
AST::arity () const
{
    return 0;
}

AST_Ptr
AST::child (int) const
{
    throw out_of_range ("node has no children");
}

AST_Vect&
AST::children ()
{
    throw logic_error ("node has no children");
}

AST_Ptr
AST::setChild (int k, AST_Ptr c)
{
    throw out_of_range ("node has no children");
}

AST_Ptr
AST::add (AST_Ptr)
{
    throw logic_error ("cannot add children to node");
}

AST_Ptr
AST::add (const AST_Vect& v)
{
    throw logic_error ("cannot add children to node");
}

bool
AST::isType ()
{
    return false;
}

Type_Ptr
AST::asType ()
{
    throw logic_error ("node does not represent a type");
}

static AST_Vect emptyVect;

AST::iterator
AST::begin ()
{
    return emptyVect.begin ();
}

AST::iterator
AST::end ()
{
    return emptyVect.end ();
}

AST::reverse_iterator
AST::rbegin ()
{
    return emptyVect.rbegin ();
}

AST::reverse_iterator
AST::rend ()
{
    return emptyVect.rend ();
}

AST_Ptr
AST::make () const
{
    throw logic_error("unknown AST type");
}

int
AST::registerExemplar (AST_Ptr node, AST_Index typeIndex)
{
    node->setTypeIndex (typeIndex);
    exemplars[typeIndex] = node;
    return 0;
}

AST::AST_Printer::AST_Printer (ostream& os, int indent)
    : os(os), indent(indent)
{ }

void
AST::findCircular (AST_Printer& printer)
{
    if (!printer.seen.emplace (this).second) {
        printer.nodeNums.emplace (this, printer.nodeNums.size () + 1);
    } else {
        for (auto c : *this) {
            c->findCircular (printer);
        }
        printer.seen.erase (this);
    }
}            

void
AST::print (ostream& os, int indent)
{
    AST_Printer printer (os, indent);
    
    findCircular (printer);
    printer.seen.clear ();
    print (printer);
}


void
AST::print (AST_Printer& printer)
{
    auto num = printer.nodeNums.find (this);
    if (num == printer.nodeNums.end () && printer.seen.count (this) == 1) {
        printer.os << "#" << num->second << "#";
        return;
    }
    else if (num != printer.nodeNums.end ()) {
        printer.seen.insert (this);
        printer.os << "#" << num->second << "=";
    }

    printer.os << "(" << astTypeName () << " " << location ();
    printer.indent += 4;
    for (auto c : *this) {
        printer.os << endl << setw (printer.indent) << "";
        c->print (printer);
    }
    printer.indent -= 4;
    printer.os << ")";
}

Decl*
AST::getDecl ()
{
    const Decl_Vect& decls = getDecls ();
    if (decls.empty ())
        return nullptr;
    return decls[0];
}

const Decl_Vect&
AST::getDecls ()
{
    // TODO: should this return an empty decl list if not decls?
    const Decl_Vect* d = new Decl_Vect;
    return *d;
    //UNIMPLEMENTED;
}

Type_Ptr
AST::getType ()
{
    if (_type == nullptr)
        _type = computeType ();
    return _type;
}

bool
AST::setType (Type_Ptr type, Unifier& subst)
{
    return unify (type, getType (), subst);
}

Type_Ptr
AST::computeType ()
{
    return newTypeVar();
    //UNIMPLEMENTED

}

int 
AST::getNumDecls ()
{
    return 0;
}

void 
AST::addDecl (Decl* decl)
{
    UNIMPLEMENTED;
}

void
AST::removeDecl (int k)
{
    UNIMPLEMENTED;
}

/** Standard processing for an outer-level statement. doOuterSemantics
 *  is recursively called only for the individual statements of a
 *  module or an outer-level statement list.*/
AST_Ptr
AST::doOuterSemantics ()
{
    AST_Ptr tree;
    tree = this;
    tree->collectDecls (mainModule);
    tree->resolveSimpleTypeIds (outerEnviron);
    tree = tree->resolveSimpleIds (outerEnviron);
    tree = tree->resolveAllocators (outerEnviron);
    tree = tree->resolveStaticSelections (outerEnviron);
    tree->resolveTypesOuter (mainModule);
    // Might need other processing as well.
    return tree;
}

void
AST::collectDecls (Decl* enclosing)
{
    //error(location(), astTypeName ());
    for (auto c : *this) {
        //error(location(),"child: %s", c->astTypeName ());
        c->collectDecls (enclosing);
    }
}

void
AST::collectTypeVarDecls (Decl* enclosing)
{
    for (auto c : *this) {
        c->collectTypeVarDecls (enclosing);
    }
}

void
AST::addTargetDecls (Decl* enclosing)
{
    for (auto c : *this) {
        c->addTargetDecls (enclosing);
    }
}

AST_Ptr
AST::resolveSimpleIds (const Environ* env)
{
    for (auto& c : *this) {
        c = c->resolveSimpleIds (env);
    }
    return this;
}

void
AST::resolveSimpleTypeIds (const Environ* env)
{
    for (auto c : *this) {
        c->resolveSimpleTypeIds (env);
    }
}
    
AST_Ptr
AST::resolveAllocators (const Environ* env)
{
    for (auto& c : *this) {
        c = c->resolveAllocators (env);
    }
    return this;
}

AST_Ptr
AST::resolveStaticSelections (const Environ* env)
{
    for (auto& c : *this) {
        c = c->resolveStaticSelections (env);
    }
    return this;
}

Decl_Set
AST::reachableDecls ()
{
    Decl_Set decls;
    decls.insert (mainModule);
    reachableDecls (decls);
    return decls;
}

void
AST::reachableDecls (Decl_Set& decls)
{
    if (this->getNumDecls() != 0) {
        for (auto& d : this->getDecls()) {
                //error(location(), d->getName().c_str());
            decls.insert(d);
        }
    }
    for (auto c : *this) {
        c->reachableDecls (decls);
    }

    //error(location(), "AST node");
    //error(location(), this->astTypeName());
    //error(location(), this->getDecl()->getName().c_str());
    //for (auto c : *this) {
        
        //error(location(),"child of module: %s", c->astTypeName ());
        // TODO: fix me 
/*         if (c->getNumDecls() != 0) {*/
            //for (auto& d : c->getDecls()) {
                ////error(location(), d->getName().c_str());
                //decls.insert(d);
            //}
            ////error(location(), "reached decl");
        //}
        /*c->reachableDecls (decls);*/
    //}


}

void
AST::freezeDecls (bool frozen)
{
    for (auto c : *this) {
        c->freezeDecls (frozen);
    }
}

AST_Ptr
AST::replaceBindings (const Unifier& subst)
{
    AST_Set visiting;

    return replaceBindings (subst, visiting);
}

AST_Ptr
AST::replaceBindings (const Unifier& subst, AST_Set& visiting)
{
    //error(location(), "%s", this->astTypeName());
    if (visiting.emplace (this).second) {
        for (auto& c : *this) {
            c = c->replaceBindings (subst, visiting);
            //error(location(), "helllo");
        }
    }
    if (_type != nullptr) {
        _type = ((AST_Ptr) _type)->replaceBindings (subst, visiting)->asType ();
    }
    return this;
}

//ADDED
void
AST::resolveDeclTypes(Unifier& subst)
{
    if(!isType() && getDecl() != nullptr){
        unify(getType(), getDecl()->getType(), subst);
        getDecl()->replaceBindings(subst);
        replaceBindings(subst);
    }
}

AST_Ptr
NODE (AST_Index typeIndex)
{
    AST_Ptr result;
    if ((unsigned int) typeIndex >= (unsigned int) LAST_AST) {
        throw out_of_range ("bad AST type index");
    }
    if (exemplars[typeIndex] == NULL) {
        result = new AST_Tree;
    } else {
        result = exemplars[typeIndex]->make();
    }
    result->setTypeIndex (typeIndex);
    return result;
}

/** A new AST leaf node, whose type is identified by TYPEINDEX.  TEXT
 *  is the semantic content of the node, and LOC its location. */
AST_Ptr
LEAF (AST_Index typeIndex, const string& text, Location loc)
{
    AST_Ptr result;
    if ((unsigned int) typeIndex >= (unsigned int) LAST_AST) {
        throw out_of_range ("bad AST type index");
    }
    if (exemplars[typeIndex] == NULL) {
        result = new AST_Tree;
    } else {
        result = exemplars[typeIndex]->make ();
    }
    result->setTypeIndex (typeIndex);
    result->setLoc (loc);
    result->setText (text);
    return result;
}


/* Definitions of methods in base class AST_Leaf. */

string
AST_Leaf::text () const
{
    return value;
}

void
AST_Leaf::setText (const string& text)
{
    value = text;
}

void
AST_Leaf::appendText (const string& text)
{
    value += text;
}

void
AST_Leaf::print (AST_Printer& printer)
{
    printer.os << "(" << astTypeName () << " " << location () << ")";
}

/* Definitions of methods in base class AST_Tree. */

/* Other support functions. */

/** Create a new ID token whose text is TEXT, and whose location is LOC. */
AST_Ptr
makeId (const char* text, Location loc)
{
    AST_Ptr result = LEAF (ID_AST, text, loc);
    result->setLoc (loc);
    return result;
}

AST_Ptr
makeId (const string& text, Location loc)
{
    AST_Ptr result = LEAF (ID_AST, text, loc);
    result->setLoc (loc);
    return result;
}


/** A node representing a missing element in the AST. */
class Empty_AST : public AST_Tree {

    /** Override AST_Tree::print */
    void print (AST_Printer& printer) override {
        printer.os << "()";
    }

};

/* Debugging utilities */

/** Output AST on the standard error output, using its print method. */
void
DB(AST* ast)
{
    if (ast == NULL) {
        cerr << "<null>" << endl;
    } else {
        ast->print(cerr, 0);
        cerr << endl;
    }
}
