# Test being able to define multiple classes.
class Foo1:
    foo1_var = 8

def foo():
    print "decoy"

class Foo2:
    foo2_var = 5

print Foo2.foo2_var
