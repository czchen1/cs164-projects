/* -*- mode: C++; c-file-style: "stroustrup"; indent-tabs-mode: nil; -*- */

/* ast.cc: Basic AST subclasses. */

/* Authors:  YOUR NAMES HERE */

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

AST::AST () : loc (0L), index (EMPTY_AST)
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

const AST_Vect*
AST::children () const
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

AST_Ptr
_Node (AST_Index typeIndex)
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

AST_Ptr
_Node (AST_Index typeIndex, AST_Ptr first, ...)
{
    Location loc;
    AST_Ptr result = _Node(typeIndex);
    if (first == NULL) {
        return result;
    }
    result->add (first);
    loc = first->location ();
    
    va_list ap;
    va_start (ap, first);
    while (true) {
        AST_Ptr arg = va_arg (ap, AST_Ptr);
        if (arg == NULL) {
            break;
        }
        result->add (arg);
        if (loc == 0) {
            loc = result->location ();
        }
    }
    va_end (ap);
    result->setLoc (loc);
    return result;
}

AST_Ptr
_Node (AST_Index typeIndex, const AST_Vect* kids, AST_Ptr)
{
    Location loc;
    loc = 0;

    AST_Ptr result = _Node (typeIndex);
    result->add (*kids);
    for_const_iter (c, *kids) {
        if (loc == 0) {
            loc = (*c)->location ();
        }
    }
    result->setLoc(loc);
    return result;
}

AST_Ptr
_Node (AST_Index typeIndex, AST_Ptr child0,
       const AST_Vect* children, AST_Ptr)
{
    AST_Ptr result = NODE (typeIndex, child0);
    result->add (*children);
    return result;
}

AST_Ptr
_Node (AST_Index typeIndex, AST_Ptr child0, AST_Ptr child1,
       const AST_Vect* children, AST_Ptr)
{
    AST_Ptr result = NODE (typeIndex, child0, child1);
    result->add (*children);
    return result;
}

AST_Ptr
_Node (AST_Index typeIndex, AST_Ptr child0, AST_Ptr child1, AST_Ptr child2,
       const AST_Vect* children, AST_Ptr)
{
    AST_Ptr result = NODE (typeIndex, child0, child1, child2);
    result->add (*children);
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
AST_Leaf::print (ostream& out, int indent)
{
    out << "(" << astTypeName () << " " << location () << ")";
}

/* Definitions of methods in base class AST_Tree. */

void
AST_Tree::print (ostream& out, int indent)
{
    out << "(" << astTypeName () << " " << location ();
    for_iter (c, *this) {
        out << endl << setw (indent + 4) << "";
        (*c)->print (out, indent + 4);
    }
    out << ")";
}

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
    void print (ostream& out, int indent) {
        out << "()";
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
    }
}
