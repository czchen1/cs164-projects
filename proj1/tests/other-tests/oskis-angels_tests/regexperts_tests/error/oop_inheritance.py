# Test that only the default parent class is allowed, so an inheritance case
# like this should error.
class Parent:
    def __init__(self):
        pass

class Foo(Parent):
    class_var = 6

    def __init__(self, x):
        self.x = x

    def make_noise(self, y):
        return y

f = Foo(7)
print Foo.class_var, f.x, f.make_noise(8)
