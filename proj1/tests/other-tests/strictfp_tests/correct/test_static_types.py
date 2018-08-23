a = "Hello"
def f(x):
    a::int = x
    print a + 3
f(3)

def incr(n::int)::int:
    return n+1
print incr(incr(incr(1)))
