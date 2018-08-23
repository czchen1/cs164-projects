# Test some more complicated constructions with static typing, like nested
# "of" constructions, function types, and classes taking in type arguments.
class Foo:
    def __init__(self):
        x::int = 4

    def complex_function(self, lst::list of [list of dict of [str, str]]):
        print "Nested typing"

    def hof(self, hof_fn::(int, int)->int)::int:
        return hof_fn(4, 5)

class Bar:
    def __init__(self):
        self.y = 3


class FooUsers of [$Foo, $Bar]:
    def __init__(self, f::$Foo, b::$Bar):
        self.f = f
        self.b = b

    def list_of_functions(self, lof):
        ret::list of [(int, list of str)->str] = lof
        to_add = (1, 2, 3, 4)
        for i::int in to_add:
            ret.append(i)
        return ret
