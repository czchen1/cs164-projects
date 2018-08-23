# Check that first parameter has proper type.

class A:
    def f(x:: str, y:: int):
        return x * y
