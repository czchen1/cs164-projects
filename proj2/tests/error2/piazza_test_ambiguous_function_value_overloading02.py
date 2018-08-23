def f():
    def g():
        return 5
    def g(x):
        return True
    return g
y = f()(3)
