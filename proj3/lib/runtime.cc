/* -*- mode: C++; c-file-style: "stroustrup"; indent-tabs-mode: nil; -*- */

/* APYC Runtime Library */

/* Authors:  YOUR NAMES HERE */

// OTHERS?

#include "runtime.h"
#include <tuple>
#include <ostream>
#include <sstream>
#include <cmath>

using namespace std;

extern char** main_argv;
extern int main_argc;
extern unordered_map<Any, int> reference_count;

// BOOLEANS

// TODO: fix both.
// Is this right???
Bool __truth__ (Any x) {
	if (x->isTrue()) {
	   return new Bool_Obj(true);
	}
	return new Bool_Obj(false);
}

// Is this right???
Bool __not__ (Any x) {
    return new Bool_Obj(!(__truth__(x)->getValue()));
//	if (!x) {
//		return new Bool_Obj(true);
//	}
//	return new Bool_Obj(false);
}

// RANGE
Range __xrange__ (Int low, Int high) {
	return new Range_Obj (low->getValue(), high->getValue());
}

Int __len__range__ (Range r) {
	return new Int_Obj (r->getLen());
}

// INTEGERS
Int __add__int__ (Int x, Int y) {
    return new Int_Obj (x->getValue() + y->getValue());
}

Int __sub__int__ (Int x, Int y) {
	return new Int_Obj(x->getValue() - y->getValue());
}

Int __mul__int__ (Int x, Int y) {
	return new Int_Obj(x->getValue() * y->getValue());
}

Int __floordiv__int__ (Int x, Int y) {
	return new Int_Obj(x->getValue() / y->getValue());
}

Int __mod__int__ (Int x, Int y) {
	return new Int_Obj(x->getValue() % y->getValue());
}

Int __pow__int__ (Int x, Int y) {
	return new Int_Obj(pow(x->getValue(), y->getValue()));
}

Int __neg__int__ (Int x) {
	return new Int_Obj(x->getValue() * -1);
}

Int __pos__int__ (Int x) {
	return new Int_Obj(x->getValue());
}

Bool __lt__int__ (Int x, Int y) {
    if (x == NULL)
        return new Bool_Obj (false);
    return new Bool_Obj (x->getValue() < y->getValue());
}

Bool __gt__int__ (Int x, Int y) {
    if (x == NULL)
        return new Bool_Obj (false);
    return new Bool_Obj (x->getValue() > y->getValue());
}

Bool __le__int__ (Int x, Int y) {
    if (x == NULL)
        return new Bool_Obj (false);
    return new Bool_Obj (x->getValue() <= y->getValue());
}

Bool __ge__int__ (Int x, Int y) {
    if (x == NULL)
        return new Bool_Obj (false);
    return new Bool_Obj (x->getValue() >= y->getValue());
}

Bool __eq__int__ (Int x, Int y) {
    if (x == NULL)
        return new Bool_Obj (false);
    return new Bool_Obj (x->getValue() == y->getValue());
}

Bool __ne__int__ (Int x, Int y) {
    if (x == NULL)
        return new Bool_Obj (false);
    return new Bool_Obj (x->getValue() != y->getValue());
}

// STRINGS
Str __add__str__ (Str x, Str y) {
    return new Str_Obj (x->getValue() + y->getValue());
}

Str __lmul__str__ (Str x, Int y) {
    string s = "";

    for (size_t i = 0; i < y->getValue(); i++) {
        s += x->getValue ();
    }
    return new Str_Obj (s);
}

Str __rmul__str__ (Int x, Str y) {
    return __lmul__str__ (y, x);
}

Bool __lt__str__ (Str x, Str y) {
    if (x == NULL)
        return new Bool_Obj (false);
    return new Bool_Obj (x->getValue().compare(y->getValue()) < 0);
}

Bool __gt__str__ (Str x, Str y) {
    if (x == NULL)
        return new Bool_Obj (false);
    return new Bool_Obj (x->getValue().compare(y->getValue()) > 0);
}

Bool __le__str__ (Str x, Str y) {
    if (x == NULL)
        return new Bool_Obj (false);
    return new Bool_Obj (x->getValue().compare(y->getValue()) <= 0);
}

Bool __ge__str__ (Str x, Str y) {
    if (x == NULL)
        return new Bool_Obj (false);
    return new Bool_Obj (x->getValue().compare(y->getValue()) >= 0);
}

Bool __eq__str__ (Str x, Str y) {
    if (x == NULL)
        return new Bool_Obj (false);
    return new Bool_Obj (x->getValue().compare(y->getValue()) == 0);
}

Bool __ne__str__ (Str x, Str y) {
    if (x == NULL)
        return new Bool_Obj (false);
    return new Bool_Obj (x->getValue().compare(y->getValue()) != 0);
}

Str __getitem__str__ (Str S, Int k) {
    return new Str_Obj (S->getValue().substr(S->ind(k->getValue()), 1));
}

Str __getslice__str__ (Str S, Int L, Int U) {
//	string slice = "";
//	for (int i = L->getValue(); i < U->getValue(); ++i){
//		slice += S->getValue().at(i);
//	}
//	return new Str_Obj (slice);
    int l = L->getValue();
    int u = U->getValue();
    return new Str_Obj (S->getValue().substr(S->ind(l), u - l));
}

Int __len__str__ (Str S) {
    return new Int_Obj (S->getLen());
}

// LISTS

Any __getitem__list__ (List S, Int k) {
    return S->getElement(k->getValue());
}

List __getslice__list__ (List S, Int L, Int U) {
    return S->getSlice (L->getValue(), U->getValue());
}

Any __setitem__list__ (List S, Int k, Any val) {
    return S->setElement(k->getValue(), val);
}

List __setslice__list__ (List S, Int a, Int b, List val) {
    return S->setSlice(a->getValue(), b->getValue(), val);
}

Int __len__list__ (List S) {
    return new Int_Obj (S->getLen());
}

// TODO: fix
// String -> Int conversion (MAYBE INCORRECT)
Int __toint__str__ (Str x) {
	return new Int_Obj (stoi(x->getValue()));
}

// Anything -> String conversion
Str __toStr__ (Any x) {
    return new Str_Obj (x->toString());
}


List __argv__ () {
    vector<Str> args;
    for (int i = 0; i < main_argc; i++) {
        //cout << to_string(i) << string(main_argv[i]) << endl;
        args.push_back (new Str_Obj (string(main_argv[i])));
        //cout << "hiiii" << endl;
    }
    return new List_Obj<Str>(args);
}

Str __readline__ () {
    string line;
    getline(cin, line);
    return new Str_Obj(line);
}

Str __read__ () {
    string single;
    single = getchar();
    return new Str_Obj(single);
}


/*template <class T>*/
//T __getitem__dict__int__ (Dict<Int, T> D, Int x) {
	//return D->getElement(x);
//}
//template <class T>
//T __getitem__dict__bool__ (Dict<Bool, T> D, Bool x) {
	//return D->getElement(x);
//}
//template <class T>
//T __getitem__dict__str__ (Dict<Str, T> D, Str x) {
	//return D->getElement(x);
/*}*/
Any __getitem__dict__int__ (Dict D, Int x) {
    return D->getElement(x);
}
Any __getitem__dict__bool__ (Dict D, Bool x) {
        return D->getElement(x);
}
Any __getitem__dict__str__ (Dict D, Str x) {
        return D->getElement(x);
}
Any __setitem__dict__int__ (Dict S, Int k, Any val) {
    return S->setElement (k, val);
}
Any __setitem__dict__bool__ (Dict S, Bool k, Any val) {
    return S->setElement (k, val);
}
Any __setitem__dict__str__ (Dict S, Str k, Any val) {
    return S->setElement (k, val);
}
Int __len__dict__ (Dict D) {
    return new Int_Obj (D->getLen ());
}
// template <class T>
// Any __setitem__dict__int__ (Dict S, Int k, Any val);
// template <class T>
// Any __setitem__dict__bool__ (Dict S, Bool k, Any val);
// template <class T>
// Any __setitem__dict__str__ (Dict S, Str k, Any val);
// Int __len__dict__int__ (Dict D);
// Int __len__dict__bool__ (Dict D);
// Int __len__dict__str__ (Dict D);

 Bool __contains__dict__int__ (Int x, Dict D) {
    return new Bool_Obj (D->in(x));
}
 Bool __contains__dict__bool__ (Bool x, Dict D){
    return new Bool_Obj (D->in(x));
}
 Bool __contains__dict__str__ (Str x, Dict D){
    return new Bool_Obj (D->in(x));
 }
 Bool __notcontains__dict__int__ (Int x, Dict D) {
    return __not__(__contains__dict__int__(x, D));
}
 Bool __notcontains__dict__bool__ (Bool x, Dict D) {
    return __not__(__contains__dict__bool__(x, D));
}
 Bool __notcontains__dict__str__ (Str x, Dict D) {
    return __not__(__contains__dict__str__(x, D));
}

Bool __is__ (Any x, Any y){
    if(x == y){
        return new Bool_Obj(true);
    }
    return new Bool_Obj(false);
}

Bool __isnot__ (Any x, Any y){
    if(x != y){
        return new Bool_Obj(true);
    }
    return new Bool_Obj(false);
}

Any __choose__list__(List L)
{
  return new Int_Obj(0);
}

Int __choose__list__(Range R)
{
  return new Int_Obj(0);
}

Any __gc__ ();

                /* OTHER RUNTIME SUPPORT (SUGGESTIONS) */

/** For implementing list displays.  __createList__(n, a1, ..., an) creates
 *  the list [a1, a2, ..., an]. */
//List __createList__ (int n, ...);

/** For implenting dictionary displays.
 *  __createDict__(n, k1, v1, k2, v2, ..., kn, vn) creates the dictionary
 *      { k1 : v1, k2 : v2, ..., kn : vn  }
 */
//Dict __createDict__ (int n, ...);

/** Allocate an object with space for N instance variables. */
Any __createObj__ (int n);
