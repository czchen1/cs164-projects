def wrapper():
    def f():
        return 5
    def f(x):
        return True
    g = f
    x = g(3)
    return x
