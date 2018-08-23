/* -*- mode: C++; c-file-style: "stroustrup"; indent-tabs-mode: nil; -*- */

/* exprs.cc: AST subclasses related to expressions. */

/* Authors:  YOUR NAMES HERE */

// The default AST printing method defined in ast.cc suffices for
// almost all ASTs.  There are a few exceptions, shown here.
// Strictly speaking, it is not necessary to define AST subclasses for
// the different kinds of AST, but you may want to anyway just to get
// a leg up on projects 2 and 3, where you will want to override
// methods to define node-specific behaviors.  The Println_AST class
// in stmts.cc is an example of a minimal AST subclass definition that
// simply inherits print and other methods unchnaged.


#include <iostream>
#include <iomanip>
#include "apyc.h"
#include "ast.h"

using namespace std;

                     /*** PRIMARY EXPRESSIONS ***/

/** Represents an integer literal. */
class Int_Literal_AST : public AST_Leaf {
private:

    void print (ostream& out, int indent) {
        out << "(int_literal " << location () << " " << value << ")";
    }

    /** Augment setText to initialize value from the text of the lexeme,
     *  checking that the literal is in range. */
    void setText (const string& text) {
	if (text.size() > 12) error(location(), "int too large");
        AST_Leaf::setText(text);
        if (this->text().compare(0, 2, "0b") == 0
         || this->text().compare(0, 2, "0B") == 0) {
            value = stol(this->text().substr(2), NULL, 2);
        } else {
            value = stol(this->text(), NULL, 0);
        }
        // FIXME: incomplete.
        if (value > 1073741824) {
            error(location(), "integer too large");
        }
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
    
    string denotedText () const {
        return literalText;
    }

    void setText (const string& text) {
        literalText.clear ();
        AST_Leaf::setText ("");
        appendText(text);
    }        

    void appendText(const string& s0) {
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

        if (s[0] == 'r' || s[0] == 'R') {
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

    void print (ostream& out, int indent) {
        out << "(string_literal " << location () << " \"";
        for (size_t i = 0; i < literalText.size (); i += 1) {
            int c = literalText[i] & 0377;
            if (c < '\040' || c == '\\' || c == '"' || c >= '\177') {
                out << "\\" << oct << setw (3) << setfill('0') << c
                    << setfill (' ') << dec;
            } else
                out << (char) c;
        }
        out << "\")";
    }

private:
    FACTORY(String_Literal_AST);

    string literalText;
};

EXEMPLAR(String_Literal_AST, STRING_LITERAL_AST);


/** Represents an identifier. **/
class Identifier_AST : public AST_Leaf {
private:
    void print (ostream& out, int indent) {
        out << "(id " << location () << " " << this->text () << ")";
    }
    FACTORY (Identifier_AST);
};
EXEMPLAR(Identifier_AST, ID_AST);

/** Represents None. Cannot be assigned to. **/
class None_AST : public AST_Leaf {
private:
    void print (ostream& out, int indent) {
        out << "(None " << location () << ")";
    }
    FACTORY (None_AST);
};
EXEMPLAR(None_AST, NONE_AST);

/** Represents True. Cannot be assigned to. **/
class True_AST : public AST_Leaf {
private:
    void print (ostream& out, int indent) {
        out << "(True " << location () << ")";
    }
    FACTORY (True_AST);
};
EXEMPLAR(True_AST, TRUE_AST);

/** Represents False. Cannot be assigned to. **/
class False_AST : public AST_Leaf {
private:
    void print (ostream& out, int indent) {
        out << "(False " << location () << ")";
    }
    FACTORY (False_AST);
};
EXEMPLAR(False_AST, FALSE_AST);

/* And operator */
class And_AST : public AST_Tree {
private:
    FACTORY (And_AST);
};
EXEMPLAR(And_AST, AND_AST);

/* Or operator */
class Or_AST : public AST_Tree {
private:
    FACTORY (Or_AST);
};
EXEMPLAR(Or_AST, OR_AST);

/* Binary operations */
class Bin_Op_AST : public AST_Tree {
private:
    FACTORY (Bin_Op_AST);
};
EXEMPLAR(Bin_Op_AST, BINOP_AST);


/* Left compare operators. Comparison operators (<, <=, ==,..) are
 * left associative: x < y< z = (x < y) < z. In addition,
 * the operators short circuit: if x < y is false, the second comparison is not made.
 * To reflect this behavior, the left compare operator is used for all 
 * comparison operators except the last, right-most one. */
class Left_Compare_AST : public AST_Tree {
private:
    FACTORY (Left_Compare_AST);
};
EXEMPLAR(Left_Compare_AST, LEFT_COMPARE_AST);

/* Comparison opert*/
class Compare_AST : public AST_Tree {
private:
    FACTORY (Compare_AST);
};
EXEMPLAR(Compare_AST, COMPARE_AST);

/* Binary operations */
class Unop_AST : public AST_Tree {
private:
    FACTORY (Unop_AST);
};
EXEMPLAR(Unop_AST, UNOP_AST);

class Subscript_AST : public AST_Tree {
public:
    FACTORY (Subscript_AST);
};
EXEMPLAR (Subscript_AST, SUBSCRIPT_AST);

class Slice_AST : public AST_Tree {
public:
    FACTORY (Slice_AST);
};
EXEMPLAR (Slice_AST, SLICE_AST);

class If_Expr_AST : public AST_Tree {
public:
    FACTORY (If_Expr_AST);
};
EXEMPLAR (If_Expr_AST, IF_EXPR_AST);

/** Represents a list.**/
class List_Display_AST : public AST_Tree {
private:
    FACTORY (List_Display_AST);
};
EXEMPLAR(List_Display_AST, LIST_DISPLAY_AST);

/** Represents a dictionary. Key value pairs in the dict are of 
 * the type Pair_AST**/
class Dict_Display_AST : public AST_Tree {
private:
    FACTORY (Dict_Display_AST);
};
EXEMPLAR(Dict_Display_AST, DICT_DISPLAY_AST);

/** Represents a pair; used to define a dict: Pair = key:value
 *  Dict = {Pair, Pair, ...} **/
class Pair_AST : public AST_Tree {
private:
    FACTORY (Pair_AST);
};
EXEMPLAR(Pair_AST, PAIR_AST);

class Call_AST : public AST_Tree {
private:
    FACTORY (Call_AST);
};
EXEMPLAR(Call_AST, CALL_AST);

class Attribute_Ref_AST : public AST_Tree {
private:
    FACTORY (Attribute_Ref_AST);
};
EXEMPLAR(Attribute_Ref_AST, ATTRIBUTEREF_AST);

