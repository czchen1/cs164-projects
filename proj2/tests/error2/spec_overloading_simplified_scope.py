def f():
    pass
def g():
    def f(x):
        pass
    f() # ERROR, the outer f is not visible.
