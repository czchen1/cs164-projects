class Foo:
    def f(self):
        return 5
foo = Foo()
def getClass():
    return Foo
print getClass().f(foo)
