def f(x, y):
    return x, y

q = f(3, "hello")
r = f(f(1, 2), "hello")
a::str, b::int = q
