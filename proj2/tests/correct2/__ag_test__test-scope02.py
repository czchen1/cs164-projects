# Simple 'def' tests

def f (x::int):
    print x

def g (y::int):
    f (y+1)

g(3)
f(3)
