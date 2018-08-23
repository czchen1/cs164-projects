def f()::int:
    return 1
def f()::str:
    return "hello"

for x in (f(), f()):
    print x
