# Make sure we know about defined methods and attributes

class A:
    y = 3

    def __init__(self):
        pass

    def f (self, x::int):
        print self.g(x)
        print self.h(), self.y

    def g (self, z::int):
        print z

    def h (self):
        q::int = self.y
        print q + 10

A().f(21)
A().g(33)
A().h()