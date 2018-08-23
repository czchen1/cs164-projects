# Test that you can't nest a class within another class.
class Outside:
    class Inside:
        def __init__(self):
            print "I am not allowed"
    x = 5
