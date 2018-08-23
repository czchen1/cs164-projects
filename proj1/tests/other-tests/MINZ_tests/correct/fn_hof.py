def make_adder(x):
    def adder(y):
        return x + y
    return adder

add_5 = make_adder(5)
print(make_adder(3))
print(make_adder(4))

def my_map(f, l):
    new_list = []
    for x in l:
        new_list = new_list + [f(x)]

def square(x):
    return x * x

print(square([-3, -2, -1, 0, 1, 2, 3]))
