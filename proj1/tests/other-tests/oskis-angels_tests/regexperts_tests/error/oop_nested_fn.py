# Test that you can't define a class inside a function.
def a_function():
    class FunctionClass:
        def __init__(self, x):
            self.x = x
    fn_class = FunctionClass(8)
    return fn_class
