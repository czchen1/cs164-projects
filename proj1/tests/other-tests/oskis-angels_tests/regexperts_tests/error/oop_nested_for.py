# Test that you can't define a class in a for loop.
for x in range(2):
    class ForClass:
        def __init__(self):
            print "I am for"
    print x
