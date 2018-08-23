def f(x::int)::int: pass
def f(x::str)::str: pass
a::(int)->int = f  # do we need to support this?
