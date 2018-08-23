# Test that you can't nest classes inside an if clause.
if True:
    class TrueClass:
        def __init__(self):
            print "I am true"
else:
    class FalseClass:
        def __init__(self):
            print "I am false"
