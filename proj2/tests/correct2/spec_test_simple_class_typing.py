class A:
    def __init__(self):
        pass

    def f(self, x::int)::bool:
        print "f(self, x::int)::bool"
        return True

x::A = A()
print A.f(x, 3)
