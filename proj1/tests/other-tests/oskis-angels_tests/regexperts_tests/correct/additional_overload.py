# Test that overloading a function with different parameters is allowed.
def foo(x, y):
    return x + y

def foo(x):
    return x * 2

def foo():
    return 4

print foo(4, 3)
print foo(4)
print foo()
