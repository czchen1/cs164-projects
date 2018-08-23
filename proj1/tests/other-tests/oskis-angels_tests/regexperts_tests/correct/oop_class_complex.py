# Test a complex class with class variables & methods as well as instance
# variables and methods. Allow for control blocks and other features in the
# class. Allow for adding members to the class

class Foo:

    def __init__(self, name, age):
        self.name = name
        self.age = age
        self.lst = [1, 2, 3]
        Foo.num_foos = Foo.num_foos + 1

    def find_in_name(self, c):
        count = 0
        for x in self.name:
            if x == c:
                count = count + 1
        return count

    num_foos = 0

    def print_num_foos(self):
        def inner_function_because_why_not():
            print Foo.num_foos
        inner_function_because_why_not()

    def countdown_age(self):
        if self.age < 0:
            return
        age = self.age
        while age > 0:
            print age
            age = age - 1

f1 = Foo("blabb", 205)
f2 = Foo("bleh", 4)
Foo.print_num_foos(f1)
print f1.find_in_name('b')
print
f2.countdown_age()
Foo.new_var = 10
print Foo.new_var
