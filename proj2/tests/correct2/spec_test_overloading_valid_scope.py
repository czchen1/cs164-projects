def f():
    print "f()"
def g():
    def f(x):
        print "f(x)"
    f(3)
g()
f()
