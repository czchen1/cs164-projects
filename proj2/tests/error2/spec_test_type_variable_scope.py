class Foo of [$T]: # Line 1
    def f(self, x::$T): # $T does not refer to the same type as Line 1
        pass
    def g(self):
        x::$T = 3 # Illegal; $T from Line 1 is not visible
