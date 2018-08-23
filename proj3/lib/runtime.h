/* -*- mode: C++; c-file-style: "stroustrup"; indent-tabs-mode: nil; -*- */

/* APYC Runtime Library Headers */

/* Authors:  YOUR NAMES HERE */

/* The functions here are suggestions. Feel free to replace at will.
 * Though this file is C++, we have avoided overloading and inheritance, so
 * that changing to C is relatively easy. */

#ifndef _RUNTIME_H_
#define _RUNTIME_H_

#include <cstdlib>
#include <iostream>
#include <tuple>
#include <vector>
#include <assert.h>
#include <unordered_map>
#include <string.h>

using namespace std;

                /* STUBS FOR BUILTIN TYPES */
class Env_Obj;
template <class T, class ...Types>
class Closure_Obj;
template <class T, class C, class ...Types>
class Closure_Method_Obj;
class Str_Obj;
class Int_Obj;
class Bool_Obj;
class Range_Obj;
template <class T>
class List_Obj;
template <class T2>
class Dict_Obj;
class Tuple0_Obj;
template <class T1>
class Tuple1_Obj;
template <class T1, class T2>
class Tuple2_Obj;
template <class T1, class T2, class T3>
class Tuple3_Obj;
class Any_Obj;
class List_Obj_Base;
class Dict_Obj_Base;

template <class T, class ...Types>
using Closure = Closure_Obj <T, Types...>*;
template <class T, class C, class ...Types>
using Closure_Method = Closure_Method_Obj <T, C, Types...>*;

typedef Env_Obj* Env;
typedef Str_Obj* Str;
typedef Int_Obj* Int;
typedef Bool_Obj* Bool;
typedef Range_Obj* Range;
typedef Tuple0_Obj* Tuple0;
template <class T1>
using Tuple1 = Tuple1_Obj<T1>*;
template <class T1, class T2>
using Tuple2 = Tuple2_Obj<T1, T2>*;
template <class T1, class T2, class T3>
using Tuple3 = Tuple3_Obj<T1, T2, T3>*;
typedef List_Obj_Base* List;
typedef Dict_Obj_Base* Dict;
typedef Any_Obj* Any;
extern unordered_map<Any, int> reference_count;

class Any_Obj {
    public:

    void* getValue () {
        return _val;
    }

    virtual bool isTrue() {
        return false;
    }

    virtual string toString () {
        return "None";
    }

    virtual string toString_quotes () {
        return toString ();
    }

    private:
    void* _val;
};
inline ostream& operator<< (ostream& out, Any_Obj* i) {
    return out << i->toString();
}

class Env_Obj : public Any_Obj {
public:
    Env_Obj (Env parent_env) {
        parent = parent_env;
    }

    Any get (string nameId) {
        map.find (nameId);
        //cout << "get " << nameId << endl;
        if (map.find (nameId) != map.end()) {
            //cout << "found item " << nameId << endl;
            return map [nameId];
        }
        else {
            //if (parent == NULL)
                //cout << "check parent" << endl;
            //else
                //cout << "null parent" << endl;
            return parent->get (nameId);
        }
    }

    Any set (string nameId, Any val) {
        map [nameId] = val;
        return val;
    }

    unordered_map<string, Any> map;
    Env parent;

};

template <class T, class ...Types>
class Closure_Obj : public Any_Obj {
public:
    Closure_Obj (T (*fp) (Types..., Env), Env env) {
        parent_env = env;
        func_ptr = fp;
    }
    T call (Types... args) {
        // TODO: fix me error
        if (func_ptr == NULL)
            cerr << "error: Null fp" << endl;
        return func_ptr (args..., parent_env);
    }
    Env parent_env;
    T (*func_ptr) (Types..., Env);
};

template <class T, class C, class ...Types>
class Closure_Method_Obj : public Any_Obj {
public:
    Closure_Method_Obj (T (C::*fp) (Types..., Env), C* instance, Env env) {
        parent_env = env;
        func_ptr = fp;
        _instance = instance;
    }
    T call (Types... args) {
        return (_instance->*func_ptr) (args..., parent_env);
    }
    Env parent_env;
    T (C::*func_ptr) (Types..., Env);
    C* _instance;
};



class Str_Obj : public Any_Obj {

public:

    Str_Obj (string value) {
        _val = value;
        cmpr = true;
    }

    string getValue () {
        return _val;
    }

    vector<Str> getVect () {
      vector<Str> temp;
      for (char &c : _val)
      {
        char *copy = new char[1];
        strncpy(copy, &c, 1);
        string *myString = new string(copy);
        temp.push_back(new Str_Obj(*myString));
      }
      return temp;
    };

    Str operator== (Str_Obj &b) {
        if (!(_val == b.getValue()) || !cmpr || !b.cmpr) {
            Str dummy = new Str_Obj("");
            dummy->cmpr = false;
            return dummy;
        }
        return &b;
    }

    Str operator!= (Str_Obj &b) {
        if (!(_val != b.getValue()) || !cmpr || !b.cmpr) {
            Str dummy = new Str_Obj("");
            dummy->cmpr = false;
            return dummy;
        }
        return &b;
    }

    Str operator< (Str_Obj &b) {
        if (!(_val < b.getValue()) || !cmpr || !b.cmpr) {
            Str dummy = new Str_Obj("");
            dummy->cmpr = false;
            return dummy;
        }
        return &b;
    }

    Str operator> (Str_Obj &b) {
        if (!(_val > b.getValue()) || !cmpr || !b.cmpr) {
            Str dummy = new Str_Obj("");
            dummy->cmpr = false;
            return dummy;
        }
        return &b;
    }

    Str operator<= (Str_Obj &b) {
        if (!(_val <= b.getValue()) || !cmpr || !b.cmpr) {
            Str dummy = new Str_Obj("");
            dummy->cmpr = false;
            return dummy;
        }
        return &b;
    }

    Str operator>= (Str_Obj &b) {
        if (!(_val >= b.getValue()) || !cmpr || !b.cmpr) {
            Str dummy = new Str_Obj("");
            dummy->cmpr = false;
            return dummy;
        }
        return &b;
    }

    int ind(int index) {
        int len = getLen();
        return ((index % len) + len) % len;
    }

    int getLen() {
        return _val.length();
    }

    bool isTrue() {
        if(getLen() == 0){
            return false;
        }
        return true;
    }

    string toString () override {
        return _val;
    }

    string toString_quotes () override {
        return "'" + _val + "'";
    }

    bool cmpr;

private:
    string _val;
};


inline ostream& operator<< (ostream& out, Str_Obj* s) {
    return out << s->getValue();
}

/* Wrapper around int */
class Int_Obj : public Any_Obj {

public:

    Int_Obj (int value) {
        //reference_count[this] = 0;
        _val = value;
        cmpr = true;
    }

    int getValue () {
        return _val;
    }

    Int operator== (Int_Obj &b) {
        if (!(_val == b.getValue()) || !cmpr || !b.cmpr) {
            Int dummy = new Int_Obj(0);
            dummy->cmpr = false;
            return dummy;
        }
        return &b;
    }

    Int operator!= (Int_Obj &b) {
        if (!(_val != b.getValue()) || !cmpr || !b.cmpr) {
            Int dummy = new Int_Obj(0);
            dummy->cmpr = false;
            return dummy;
        }
        return &b;
    }

    Int operator< (Int_Obj &b) {
        if (!(_val < b.getValue()) || !cmpr || !b.cmpr) {
            Int dummy = new Int_Obj(0);
            dummy->cmpr = false;
            return dummy;
        }
        return &b;
    }

    Int operator> (Int_Obj &b) {
        if (!(_val > b.getValue()) || !cmpr || !b.cmpr) {
            Int dummy = new Int_Obj(0);
            dummy->cmpr = false;
            return dummy;
        }
        return &b;
    }

    Int operator<= (Int_Obj &b) {
        if (!(_val <= b.getValue()) || !cmpr || !b.cmpr) {
            Int dummy = new Int_Obj(0);
            dummy->cmpr = false;
            return dummy;
        }
        return &b;
    }

    Int operator>= (Int_Obj &b) {
        if (!(_val >= b.getValue()) || !cmpr || !b.cmpr) {
            Int dummy = new Int_Obj(0);
            dummy->cmpr = false;
            return dummy;
        }
        return &b;
    }

    bool isTrue() {
        if(getValue() == 0){
            return false;
        }
        return true;
    }


    string toString () override {
        return to_string(_val);
    }

    bool cmpr;

private:
    int _val;
};


class Bool_Obj : public Any_Obj {

public:

    Bool_Obj (bool value) {
        _val = value;
    }

    Bool_Obj (Int value) {
        _val = value->cmpr;
    }

    Bool_Obj (Str value) {
        _val = value->cmpr;
    }

    bool getValue () {
        return _val;
    }
    bool isTrue() {
        if(getValue()){
            return true;
        }
        return false;
    }

/*    operator bool() { */
        //return isTrue ();
    /*}*/

    string toString () override {
        if (_val)
            return "True";
        else
            return "False";
    }

private:
    bool _val;
};


class List_Obj_Base : public Any_Obj {
    public:
    List_Obj_Base () {

    }
    List_Obj_Base (vector<Any> list) {
         _val = list;
    }

    int ind(int index) {
        int len = getLen();
        return ((index % len) + len) % len;
    }

    virtual Any getElement (int index) {
        if (index >= getLen()) {
            cerr << "error : index out of bounds" << endl;
            exit(1);
        }
        return _val.at(ind(index));
    }

    virtual Any setElement (int index, Any value) {
        if (index >= getLen()) {
            cerr << "error : index out of bounds" << endl;
            exit(1);
        }
        _val.at(ind(index)) = value;
        return value;
    }

    virtual int getLen () {
        return _val.size();
    }

    virtual vector<Any> getValue () {
       return _val;
    };

    vector<Any> getVect () {
       return getValue();
    };

    virtual List getSlice (int start, int end) {
      start = max (0, start);
      end = min(min(end, getLen()), max(start, getLen()));
      vector<Any> NewVec (_val.begin() + start, _val.begin() + end);
      return new List_Obj_Base (NewVec);
    };

    virtual List setSlice (int start, int end, List new_slice) {
      if (start >= getLen())
      {
        for (int i = 0; i < new_slice->getLen(); i++) {
          _val.push_back(new_slice->getElement(i));
        }
        return new_slice;
      }
      if (end - start > 0)
      {
        end  = min(getLen(), end);
        _val.erase(_val.begin()+start, _val.begin()+end);
        for (int i = 0; i < new_slice->getLen(); i++) {
          _val.insert(_val.begin()+start+i, new_slice->getElement(i));
        }
      }
      else
      {
        for (int i = 0; i < new_slice->getLen(); i++) {
          _val.insert(_val.begin()+start+i, new_slice->getElement(i));
        }
      }
      // TODO: pass by value or reference
      return new_slice;
    };

    string toString () {
        string out = "[";
        if (getLen () > 0) {
            out += getElement(0)->toString();
        }
        for (int i = 1; i < getLen(); i++) {
            out += ", ";
            out += getElement(i)->toString();
        }
        out += "]";
        return out;
    }

    vector<Any> _val;
};


template<class T>
class List_Obj : public List_Obj_Base {

public:

    List_Obj (vector<T> list) {
        vector<Any> temp;
        for (T x : list)
        {
          temp.push_back(static_cast<Any>(x));
        }
        _val = temp;
        //reference_count[this] = 0;
    }

    T getElement (int index) override {
        if (index >= getLen()) {
            cerr << "error : index out of bounds" << endl;
            exit(1);
        }
        return static_cast<T> (_val.at(ind(index)));
    }

    T setElement (int index, Any value) override {
        if (index >= getLen()) {
            cerr << "error : index out of bounds" << endl;
            exit(1);
        }
        _val.at(ind(index)) = static_cast<T> (value);
        return static_cast<T> (value);
    }

    // List getSlice (int start, int end) {
    //     start = max (0, start);
    //     end = min (getLen(), end);
    //     vector<T> NewVec (_val.begin() + start, _val.begin() + end);
    //     return new List_Obj (NewVec);
    // }
    //
    // List setSlice (int start, int end, List new_slice) {
    //     start = max (0, start);
    //     end = min (getLen(), end);
    //
    //     assert (end-start == new_slice->getLen());
    //     for (int i = start; i < end; i++) {
    //         // TODO: should copy primitives ???
    //         setElement (i, new_slice->getElement(i));
    //     }
    //     // TODO: pass by value or reference
    //     return new_slice;
    //
    // }

    int getLen () override {
        return _val.size();
    }
    bool isTrue() {
        if(getLen() == 0){
            return false;
        }
        return true;
    }

    string toString () override {
        string out = "[";
        if (getLen () > 0) {
            out += getElement(0)->toString();
        }
        for (int i = 1; i < getLen(); i++) {
            out += ", ";
            out += getElement(i)->toString();
        }
        out += "]";
        return out;
    }

// private:
//     vector<T> _val;
};

class Range_Obj : public List_Obj_Base {

public:

    Range_Obj (int low, int high) {
        _lowVal = low;
        _highVal = high;
        vector<Any> temp;
        for (int i = low; i < high; i++)
        {
          temp.push_back(static_cast<Any>(new Int_Obj(i)));
        }
        _val = temp;
    }

    bool isTrue() {
        if(getLen() == 0){
            return false;
        }
        return true;
    }

    int getLen () override {
        return _highVal - _lowVal;
    }

    string toString () override {
        return "xrange(" + to_string(_lowVal) + ", " + to_string(_highVal) + ")";
    }

private:
    int _lowVal;
    int _highVal;
};


class Dict_Obj_Base : public Any_Obj {

public:

    virtual Any getElement(Any x) {
        // TODO: check if key in dict??
        return _dict[x];
    }

    virtual Any setElement (Any x, Any val) {
        _dict[x] = val;
        return val;
    }

    virtual bool in (Any x) {
        return false;
    }

    virtual int getLen () {
        return _dict.size();
    }

    virtual bool isTrue() {
        if(getLen() == 0){
            return false;
        }
        return true;
    }

    vector<Any> getVect () {
      vector<Any> temp;
      if (_dict_int.size() > 0)
      {
        for (auto it : _dict_int)
        {
          temp.push_back(new Int_Obj(it.first));
        }
      }
      else if (_dict_bool.size() > 0)
      {
        for (auto it : _dict_bool)
        {
          temp.push_back(new Bool_Obj(it.first));
        }
      }
      else
      {
        for (auto it : _dict_str)
        {
          temp.push_back(new Str_Obj(it.first));
        }
      }
      return temp;
    };

    unordered_map<int, Any > _dict_int;
    unordered_map<bool, Any > _dict_bool;
    unordered_map<string, Any > _dict_str;
    unordered_map<Any, Any> _dict;
};

template <class T2>
class Dict_Obj_Int : public Dict_Obj_Base {

public:

    Dict_Obj_Int (vector<pair<Int, T2>> dict) {
         for (std::pair<Int, T2> element : dict) {
             _dict[element.first->getValue()] = element.second;
             _dict_int[element.first->getValue()] = static_cast<Any>(element.second);
        }
    }

    T2 getElement(Any x) override {
        if (_dict.find (static_cast<Int>(x)->getValue()) == _dict.end())
            cerr << "error: cannot find key" << endl;
        return _dict[static_cast<Int>(x)->getValue()];
    }

    T2 setElement (Any x, Any val) override {
        _dict[static_cast<Int>(x)->getValue()] = static_cast<T2>(val);
        return static_cast<T2>(val);
    }

    bool in (Any x) override {
        return bool(_dict.find (static_cast<Int>(x)->getValue()) != _dict.end());
    }


    string toString () override {
        string out = "{";
        for (std::pair<int, T2> element : _dict) {
            out += to_string (element.first);
            out += ": ";
            out += element.second->toString_quotes ();
            out += ", ";

        }
        if (getLen() > 0) {
            out = out.substr (0, out.length()-2);
        }

        return out + "}";
    }

    int getLen () override {
        return _dict.size();
    }
    bool isTrue() override {
        if(getLen() == 0){
            return false;
        }
        return true;
    }

    unordered_map<int, T2 > _dict;
};

template <class T2>
class Dict_Obj_Bool : public Dict_Obj_Base {

public:

    Dict_Obj_Bool (vector<pair<Bool, T2>> dict) {
         for (std::pair<Bool, T2> element : dict) {
             _dict[element.first->getValue()] = element.second;
             _dict_bool[element.first->getValue()] = static_cast<Any>(element.second);
        }
    }

    T2 getElement(Any x) override {
        if (_dict.find (static_cast<Bool>(x)->getValue()) == _dict.end())
            cerr << "error: cannot find key" << endl;
        return _dict[static_cast<Bool>(x)->getValue()];
    }

    T2 setElement (Any x, Any val) override {
        _dict[static_cast<Bool>(x)->getValue()] = static_cast<T2>(val);
        return static_cast<T2>(val);
    }

    bool in (Any x) override {
        return (_dict.find (static_cast<Bool>(x)->getValue()) != _dict.end());
    }

    string toString () override {
        string out = "{";
        for (std::pair<bool, T2> element : _dict) {
            out += (element.first) ? "True" : "False";
            out += ": ";
            out += element.second->toString_quotes ();
            out += ", ";
        }
        if (getLen() > 0)
            out = out.substr (0, out.length()-2);
        return out + "}";
    }

    int getLen () override {
        return _dict.size();
    }

    bool isTrue() override {
        if(getLen() == 0){
            return false;
        }
        return true;
    }

    unordered_map<bool, T2 > _dict;
};

template <class T2>
class Dict_Obj_Str : public Dict_Obj_Base {

public:

    Dict_Obj_Str (vector<pair<Str, T2>> dict) {
         for (std::pair<Str, T2> element : dict) {
             _dict[element.first->getValue()] = element.second;
             _dict_str[element.first->getValue()] = static_cast<Any>(element.second);
        }
    }

    T2 getElement(Any x) override {
        if (_dict.find (static_cast<Str>(x)->getValue()) == _dict.end())
            cerr << "error: cannot find key" << endl;
        return _dict[static_cast<Str>(x)->getValue()];
    }

    T2 setElement (Any x, Any val) override {
        _dict[static_cast<Str>(x)->getValue()] = static_cast<T2>(val);
        return static_cast<T2>(val);
    }

    bool in (Any x) override {
        return (_dict.find (static_cast<Str>(x)->getValue()) != _dict.end());
    }

    string toString () override {
        string out = "{";
        for (std::pair<string, T2> element : _dict) {
            out += "'" + element.first + "'";
            out += ": ";
            out += element.second->toString_quotes ();
            out += ", ";
        }
        if (getLen() > 0)
            out = out.substr (0, out.length()-2);

        return out + "}";
    }

    int getLen () override {
        return _dict.size();
    }

    bool isTrue() override {
        if(getLen() == 0){
            return false;
        }
        return true;
    }
    unordered_map<string, T2 > _dict;
};



class Tuple0_Obj : public Any_Obj {

public:
    Tuple0_Obj () {
        _tuple = make_tuple ();
    }

    tuple<> getValue () {
        return _tuple;
    }

    bool isTrue() {
        return false;
    }

    string toString () override {
        return "()";
    }

private:
    tuple<> _tuple;
};
inline ostream& operator<< (ostream& out, Tuple0 t) {
    auto tuple = t->getValue();
    out << "(" << ")";
    return out;
}

template <class T1>
class Tuple1_Obj : public Any_Obj {

public:
    Tuple1_Obj (T1 val1) {
        _tuple = make_tuple (val1);
    }

    tuple<T1> getValue () {
        return _tuple;
    }

    bool isTrue() {
        return true;
    }

    string toString () override {
        return "(" + get<0>(_tuple)->toString_quotes() + ")";
    }

private:
    tuple<T1> _tuple;
};

template <class T1, class T2>
class Tuple2_Obj : public Any_Obj {

public:
    Tuple2_Obj (T1 val1, T2 val2) {
        _tuple = make_tuple (val1, val2);
    }

    tuple<T1, T2> getValue () {
        return _tuple;
    }
    bool isTrue() {
        return true;
    }

    string toString () override {
        return "(" + get<0>(_tuple)->toString_quotes() + ", " + get<1>(_tuple)->toString_quotes()  + ")";
    }

private:
    tuple<T1, T2> _tuple;
};

template <class T1, class T2, class T3>
class Tuple3_Obj : public Any_Obj {

public:
    Tuple3_Obj (T1 val1, T2 val2, T3 val3) {
        _tuple = make_tuple (val1, val2, val3);
    }

    tuple<T1, T2, T3> getValue () {
        return _tuple;
    }
    bool isTrue() {
        return true;
    }

    string toString () override {
        return "(" + get<0>(_tuple)->toString_quotes() + ", " + get<1>(_tuple)->toString_quotes() + ", " + get<2>(_tuple)->toString_quotes()  + ")";
    }

private:
    tuple<T1, T2, T3> _tuple;
};


                /* NATIVE METHODS */
/* Function names are those used in 'native' statements. */

/*  Type Bool */

extern Bool __truth__ (Any x);
extern Bool __not__ (Any x);

/** The values of True and False. */
extern Bool __true__, __false__;

/*  Type Range */

extern Range __xrange__ (Int low, Int high);
extern Int __len__range__ (Range r);

/*  Type Int */

extern Int __add__int__ (Int x, Int y);
extern Int __sub__int__ (Int x, Int y);
extern Int __mul__int__ (Int x, Int y);
extern Int __floordiv__int__ (Int x, Int y);
extern Int __mod__int__ (Int x, Int y);
extern Int __pow__int__ (Int x, Int y);
extern Int __neg__int__ (Int x);
extern Int __pos__int__ (Int x);
extern Bool __lt__int__ (Int x, Int y);
extern Bool __gt__int__ (Int x, Int y);
extern Bool __le__int__ (Int x, Int y);
extern Bool __ge__int__ (Int x, Int y);
extern Bool __eq__int__ (Int x, Int y);
extern Bool __ne__int__ (Int x, Int y);

/*  toint is the replacement for 'Int' as a function in our dialect. */
extern Int __toint__str__ (Str x);

/*  Type Str */

extern Str __add__str__ (Str x, Str y);
extern Str __lmul__str__ (Str x, Int y);
extern Str __rmul__str__ (Int x, Str y);
extern Bool __lt__str__ (Str x, Str y);
extern Bool __gt__str__ (Str x, Str y);
extern Bool __le__str__ (Str x, Str y);
extern Bool __ge__str__ (Str x, Str y);
extern Bool __eq__str__ (Str x, Str y);
extern Bool __ne__str__ (Str x, Str y);
extern Str __getitem__str__ (Str S, Int k);
extern Str __getslice__str__ (Str S, Int L, Int U);
extern Int __len__str__ (Str S);

/* toStr is the replacement for 'Str' as a function in our dialect. */
extern Str __toStr__ (Any x);

/*  Type List */

//template <class T>
extern Any __getitem__list__ (List S, Int k);
extern List __getslice__list__ (List S, Int L, Int U);
extern Any __setitem__list__ (List S, Int k, Any val);
extern List __setslice__list__ (List S, Int a, Int b, List val);
extern Int __len__list__ (List S);


/*  Replaces sys.argv */
extern List __argv__ ();

/*  Type File */

extern Str __readline__ ();
extern Str __read__ ();

/*  Type Dict */
/*template <class T>*/
//extern T __getitem__dict__int__ (Dict<Int, T> D, Int x);
//template <class T>
//extern T __getitem__dict__bool__ (Dict<Bool, T> D, Bool x);
//template <class T>
/*extern T __getitem__dict__str__ (Dict<Str, T> D, Str x);*/
extern Any __getitem__dict__int__ (Dict D, Int x);
extern Any __getitem__dict__bool__ (Dict D, Bool x);
extern Any __getitem__dict__str__ (Dict D, Str x);
extern Any __setitem__dict__int__ (Dict S, Int k, Any val);
extern Any __setitem__dict__bool__ (Dict S, Bool k, Any val);
extern Any __setitem__dict__str__ (Dict S, Str k, Any val);
extern Int __len__dict__ (Dict D);

/*  Backwards from regular Python */

 extern Bool __contains__dict__int__ (Int x, Dict D);
 extern Bool __contains__dict__bool__ (Bool x, Dict D);
 extern Bool __contains__dict__str__ (Str x, Dict D);

/*  Backwards from regular Python */

 extern Bool __notcontains__dict__int__ (Int x, Dict d);
 extern Bool __notcontains__dict__bool__ (Bool x, Dict d);
 extern Bool __notcontains__dict__str__ (Str x, Dict d);

/*  General */

extern Bool __is__ (Any x, Any y);
extern Bool __isnot__ (Any x, Any y);

extern Any  __choose__list__(List L);;
extern Int __choose__list__(Range R);
extern Any __choose__dict__(Dict D);
extern Any __choose__str__(Str x);

/*  Extra Credit */
extern Any __gc__ ();


                /* OTHER RUNTIME SUPPORT (SUGGESTIONS) */

/** For implementing list displays.  __createList__(n, a1, ..., an) creates
 *  the list [a1, a2, ..., an]. */
/*extern List __createList__ (int n, ...);*/

/** For implenting dictionary displays.
 *  __createDict__(n, k1, v1, k2, v2, ..., kn, vn) creates the dictionary
 *      { k1 : v1, k2 : v2, ..., kn : vn  }
 */
//extern Dict __createDict__ (int n, ...);

/** Allocate an object with space for N instance variables. */
extern Any __createObj__ (int n);

#endif
