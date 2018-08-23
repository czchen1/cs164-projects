def doprint(x):
    print(x)

myprint = print

myprint("hello")
doprint("world")

def factorial(i :: int ) :: int:
    if i == 0:
        return 1
    else:
        return i * factorial(i - 1)

print(factorial(1))
print(factorial(2))
print(factorial(3))
print(factorial(4))
print(factorial(5))
