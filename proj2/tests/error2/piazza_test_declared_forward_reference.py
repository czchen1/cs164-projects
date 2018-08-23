class A:
    def f(self, x::B):
        return x.g(3)

class B:
    def g(self, x):
        return x
