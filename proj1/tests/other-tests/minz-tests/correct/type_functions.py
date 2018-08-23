def inc(x :: int) :: int:
    return x + 1

def add(x :: int, y :: int) :: int:
    return x + y

def curry(f :: (int, int) -> int, x :: int) :: (int) -> int:
    def newf(y :: int) :: int:
        return f(x, y)
    return newf

add5 = curry(add, 5)
print(add5(6))
