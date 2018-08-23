def make_adder(x):
    def adder(y):
        return x + y
    return adder

add_5 = make_adder(5)
print(add_5(3))
print(add_5(4))

def my_map(f, l):
    new_list = []
    for x in l:
        new_list = new_list + [f(x)]
    return new_list

def square(x):
    return x * x

print(my_map(square, [-3, -2, -1, 0, 1, 2, 3, 4]))
