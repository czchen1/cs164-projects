class Dog:
    def __init__(self, name):
        print "I have been named", name

    def bark(self, loud):
        i = 0
        while i < loud:
            print "bark"
            i = i + 1
