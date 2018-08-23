class A:
    class B:
        pass

if 1:
    class A:
        pass
while 1:
    class A:
        pass
    break
for _ in [1]:
    class A:
        pass

def foo():
    class A:
        pass
    return A

foo()
