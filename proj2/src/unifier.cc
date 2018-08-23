/* -*- mode: C++; c-file-style: "stroustrup"; indent-tabs-mode: nil; * -*- */

/* unifier.cc: Implementation of the Unifier class. */

#include "unifier.h"
#include <iostream>

using namespace std;

Type_Ptr
Unifier::operator[] (Type_Ptr obj) const
{
    auto p = binding.find (obj);
    if (p == binding.end () || p->second == obj) {
        return obj;
    } else {
        p->second = (*this)[p->second];
        return p->second;
    }
}

Unifier&
Unifier::bind (Type_Ptr obj, Type_Ptr target)
{
    auto r = binding.emplace (obj, target);
    if (!r.second && r.first->second != obj) {
        throw logic_error ("type object already bound");
    }
    return *this;
}

void
Unifier::swap (Unifier& other)
{
    binding.swap (other.binding);
}

void
Unifier::print () {
    cout << "unifier" << endl;
    for ( auto it = binding.begin(); it != binding.end(); ++it ) {
        cout << "Binding" << endl;
        it->first->print(cout, 0);
        cout << endl;
        it->second->print(cout, 0);
        cout << endl;
    }
}
