/* -*- mode: C++; c-file-style: "stroustrup"; indent-tabs-mode: nil; -*- */

/* ast.h: Top-level definitions for abstract syntax trees. */

/* Authors: */

#ifndef _AST_H_
#define _AST_H_

#include <string>
#include <vector>

class AST;
class AST_Leaf;
class AST_Tree;
class Type;

typedef AST* AST_Ptr;
typedef Type* Type_Ptr;

/** A vector of AST pointers. */
typedef std::vector<AST_Ptr> AST_Vect;

/** A type standing for source locations.  Source locations are
 *  represented as artificial line numbers. For locations in the first
 *  source file processed, the Location values correspond to the
 *  actual line numbers, starting from 1.  Line numbers then continue
 *  increasing in subsequent files, so that line 1 of the second file
 *  is one greater than the last line of the first.  The functions
 *  locationFileName and locationLine convert Locations back to actual
 *  line numbers and file locations. */
typedef long Location;

/** An enumeration type giving symbolic names of integer indices
 *  representing all AST types. Used for the node factories. */
enum AST_Index {

#define AST_INFO(SYM, NAME) SYM,
#include "ast-types.i"
#undef AST_INFO

};

/** The base type of all nodes in the abstract syntax tree. */
class AST {
    friend AST_Ptr _Node (AST_Index typeIndex);
    friend AST_Ptr LEAF (AST_Index typeIndex, const std::string& text,
                         Location loc);
public:
    /** The encoded location associated with this AST. */
    virtual Location location () const;

    /** Set location () to LOC. */
    virtual void setLoc (Location loc);

    /** Return the identifying syntactic category associated with this
     *  node.  This category is used by AST node factories to select
     *  the subtype of AST to use and to access other information
     *  about the node. */
    virtual AST_Index typeIndex () const;
    
    /** Set typeIndex() to SYN. */
    virtual void setTypeIndex (AST_Index syn);

    /** External name of this node's type, as printed by print. */
    virtual const char* astTypeName () const;

    /** My text content.  Generally applicable to leaf nodes only. */
    virtual std::string text () const;

    /** Set text () to TEXT. */
    virtual void setText (const std::string& text);

    /** Append S to the value of text (), if allowed. */
    virtual void appendText (const std::string& s);

    /** The text denoted by THIS, if it is a string literal.  This may
     *  differ from text () in that escape sequences have been replaced
     *  with the actual characters they denote. */
    virtual std::string denotedText () const;

    /** Print my representation as an AST on OUT.  Use INDENT as the
     *  indentation for subsequent lines if my representation takes up
     *  multiple lines. */
    virtual void print (std::ostream& out, int indent) = 0;

    /** For nodes that represent types, return the node with a static
     *  type that reveals its Type operations.  An error on node types
     *  that do not represent types. */
    virtual Type_Ptr asType ();

    /** The number of my children. */
    virtual int arity () const;

    /** My Kth child, numbering from 0. */
    virtual AST_Ptr child (int k) const;

    /** Set my Kth child to C, returning me. */
    virtual AST_Ptr setChild (int k, AST_Ptr c);

    /** My children, as a vector. */
    virtual const AST_Vect* children () const;
    
    /** Add C as a new rightmost child, returning me. */
    virtual AST_Ptr add (AST_Ptr c);

    /** Add the elements of V as new rightmost children, returning me. */
    virtual AST_Ptr add (const AST_Vect& v);

    /* Methods and types to make ASTs look like STL containers whose
     * contents are the child nodes.  (See also for_iter, etc.). */

    typedef AST_Vect::iterator iterator;
    typedef AST_Vect::reverse_iterator reverse_iterator;

    /** A forward iterator through my children. */
    virtual iterator begin ();
    /** A forward iterator to the end of my children. */
    virtual iterator end ();

    /** A reverse iterator through my children. */
    virtual reverse_iterator rbegin ();
    /** A reverse iterator to the beginning of my children. */
    virtual reverse_iterator rend ();

    /* End of container definitions. */

    /** Set NODE's type index to TYPEINDEX and register it as an
     *  exemplar.  Returns an arbitrary integer so that it may be used
     *  in an initializer. */
    static int registerExemplar (AST_Ptr node, AST_Index typeIndex);

protected:

    /** Normally, leave node creation to the factory methods Node and
     *  Leaf. */
    AST ();

    /* The following definition is for implementing a "node factory"
     * function.  The idea is to create one "exemplar" object
     * of each subclass of AST, and to place them in an array indexed
     * by a syntactic category index, unique to each type.  The allows one to
     * write general functions (see Node and Leaf) that, given a
     * syntactic category and sequence of child nodes, can produce a
     * node of the appropriate type by simply looking up the
     * appropriate exemplar by the syntactic category and then calling
     * these methods.
     *
     * See also the macros FACTORY and EXEMPLAR below.
     */

    /** Create a node of my type with no children.  Must be overridden
     *  in each subtype that wants to have an exemplar.  The intent is
     *  that the bodies of such an overriding in class T return new T. */
    virtual AST_Ptr make () const;

private:

    /** My source location. */
    Location loc;
    /** My syntactic index. */
    AST_Index index;
};

/* Factory macros.  See the definition of Module_AST in stmts.cc, for
 * example, for how to use FACTORY and EXEMPLAR to "register" a class
 * with the system so that an instance of the class may be created by
 * referring to its AST_Index.  */

/** Use this in the class declaration of TYPE to define an appropriate make()
 *  function. */
#define FACTORY(TYPE) TYPE* make () const { return new TYPE; }

/** Use this to create an exemplar for node type TYPE with the given TYPEINDEX.
 *  It creates a dummy static variable and initializes it in a way that 
 *  creates the exemplar.  */
#define EXEMPLAR(TYPE, TYPEINDEX)                                       \
    static int TYPEINDEX ## _EXEMPLAR = AST::registerExemplar(new TYPE, TYPEINDEX)


class AST_Leaf : public AST {
public:
    
    std::string text () const;

    void setText (const std::string& text);

    void appendText (const std::string& text);

    void print (std::ostream& out, int indent);

private:

    std::string value;

};
    
class AST_Tree : public AST {
public:

    /** Overrides AST::print.  Default definition of printing on
     *  trees: prints (<OP> <LINE> <CHILD0> ...), where <OP> is the 
     *  external operator name and line is the source line number. */
    void print (std::ostream& out, int indent);

    int arity () const {
        return kids.size();
    }

    AST_Ptr child (int k) {
        return kids.at(k);
    }

    AST_Ptr setChild (int k, AST_Ptr c) {
        kids.at(k) = c;
        return this;
    }

    const AST_Vect* children () const {
        return &kids;
    }
    
    AST_Ptr add (AST_Ptr c) {
        kids.push_back (c);
        return this;
    }

    AST_Ptr add (const AST_Vect& v) {
        kids.insert (kids.end (), v.begin (), v.end ());
        return this;
    }

    iterator begin() {
        return kids.begin();
    }

    iterator end() {
        return kids.end();
    }

    reverse_iterator rbegin() {
        return kids.rbegin();
    }

    reverse_iterator rend() {
        return kids.rend();
    }

    AST_Ptr make () const {
        return new AST_Tree;
    }

private:

    AST_Vect kids;

};

/** The supertype of all tree nodes that represent types.  We could
 *  instead define type-specific operations in AST, but that clutters
 *  ASTs with operations that apply only to certain subtypes.  This
 *  class is intended to factor out the operations specific to types. */
class Type : public AST_Tree {
public:

    /** Overrides AST::asType. Effectively reveals the Type-specific
     *  operations on this node.  */
    Type_Ptr asType () {
        return this;
    }

};

/** NODE(TYPEINDEX, ...) creates a new node whose type index is
 *  TYPEINDEX and whose children are given in the trailing arguments.
 *  The children may either be a sequence of 0 or more AST_Ptrs or
 *  a single AST_Vect*.  The latter has the same effect as would
 *  listing its members in place of ....  This is a macro; it uses the
 *  overloaded _Node functions, and adds a 0 parameter to the end,
 *  which is how C/C++ mark the end of a variadic parameter list
 *  (denoted "...").  You should use NODE in preference to _Node. */
#define NODE(...)  _Node (__VA_ARGS__, (AST_Ptr) 0)

/** A new AST leaf node, whose type is identified by TYPEINDEX.  TEXT
 *  is the semantic content of the node. LOC is its source location. */
extern AST_Ptr LEAF (AST_Index typeIndex, const std::string& text,
                     Location loc = 0);

/* The following are intended to be called through the NODE macro. */

/** A new AST tree node whose type is identified by TYPEINDEX.  The
 *  trailing arguments are the (initial) children. The last argument
 *  must be (to signal the end of the list). */
extern AST_Ptr _Node (AST_Index typeIndex, AST_Ptr first, ...);

/** A new AST tree node whose type is identified by TYPEINDEX, and
 *  whose children are contained in *CHILDREN. */
extern AST_Ptr _Node (AST_Index typeIndex, const AST_Vect* children, AST_Ptr);

/** A new AST tree node whose type is identified by TYPEINDEX, and
 *  whose children are CHILD0 and those contained in *CHILDREN. */
extern AST_Ptr _Node (AST_Index typeIndex, AST_Ptr child0,
                      const AST_Vect* children, AST_Ptr);

/** A new AST tree node whose type is identified by TYPEINDEX, and
 *  whose children are CHILD0, CHILD1, and those contained in *CHILDREN. */
extern AST_Ptr _Node (AST_Index typeIndex, AST_Ptr child0, AST_Ptr child1,
                      const AST_Vect* children, AST_Ptr);

/** A new AST tree node whose type is identified by TYPEINDEX, and
 *  whose children are CHILD0, CHILD1, and those contained in *CHILDREN. */
extern AST_Ptr _Node (AST_Index typeIndex, AST_Ptr child0, AST_Ptr child1,
                      AST_Ptr child2, const AST_Vect* children, AST_Ptr);

/* Other functions. */

/** Create a new ID token whose text is TEXT, and whose location is LOC. */
extern AST_Ptr makeId (const char* text, Location loc);
extern AST_Ptr makeId (const std::string& text, Location loc);


/** Used below. */
template<class T> struct unref     { typedef T type; };
template<class T> struct unref<T&> { typedef T type; };

/** Control structure:
 *     for_iter(VAR, SOURCE) {
 *        body
 *     }
 *  iterates through SOURCE (a standard STL containing iterator,
 *  defining type iterator, and methods begin() and end()), setting
 *  *VAR to each of the elements in turn.
 */
#define for_iter(VAR, SOURCE) \
    for (unref<decltype(SOURCE)>::type::iterator VAR = (SOURCE).begin(); \
         VAR != (SOURCE).end(); VAR += 1)

/** As for for_iter, but using the const_iterator of SOURCE (so that
 *  *VAR is not modifiable.) */
#define for_const_iter(VAR, SOURCE) \
    for (unref<decltype(SOURCE)>::type::const_iterator VAR = (SOURCE).begin(); \
         VAR != (SOURCE).end(); VAR += 1)

#define for_iter_reverse(VAR, SOURCE) \
    for (unref<decltype(SOURCE)>::type::reverse_iterator VAR = (SOURCE).rbegin(); \
         VAR != (SOURCE).rend(); VAR += 1)

/** As for for_iter, but using the const_iterator of SOURCE (so that
 *  *VAR is not modifiable.) */
#define for_const_iter_reverse(VAR, SOURCE) \
    for (unref<decltype(SOURCE)>::type::const_reverse_iterator VAR = (SOURCE).rbegin(); \
         VAR != (SOURCE).rend(); VAR += 1)

#endif

