def twice(f):
    def wrapper():
        print "Decorate"
        f()
    return wrapper

@twice
def foo():
    print "Hello, world"

foo()
