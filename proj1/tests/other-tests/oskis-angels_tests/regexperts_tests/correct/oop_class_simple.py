# Test a simple class with one method and one instance variable.
# Make sure that instantiating that class and accessing its members works.

class Foo:
    class_var = 6

    def __init__(self, x):
        print

    def make_noise(self, y):
        return y

f = Foo(7)
