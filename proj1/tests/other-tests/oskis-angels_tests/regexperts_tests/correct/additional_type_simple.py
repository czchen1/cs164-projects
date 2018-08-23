# Test some simple applications of static typing in function parameters and in
# other variables. Test both using identifiers as type and using type variables
# prepended with '$'.
class Foo:
    def bar(self,x::int):
        print x, "is an int"

def something(x, y::double, f::$Foo):
    newFoo::$Foo = Foo()
    newFoo = Foo()
    return f

for i::int in range(3):
    print i
