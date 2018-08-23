def func(x::str)::str:
    return "hello"

def func(x::int):
    return 5

def func(x::int, y::str):
    return y

def func(x)::bool:
    return x

a = func("a")
b = func(5)
c = func(5, "h")
d = func(True)
