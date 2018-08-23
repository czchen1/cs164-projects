# Test that you can't define a class in a while loop
x = 0
while x < 2:
    class While:
        def __init__(self):
            self.w = 3
    x = x + 1
