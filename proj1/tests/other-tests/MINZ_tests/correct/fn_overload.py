def add(x :: int, y :: int) :: int:
    print("adding ints")
    return x + y

def add(x :: str, y :: str) :: str:
    print("adding strs")
    return x + y

print(add(2, 3))
print(add("hello", "world"))
