class A:
    x = 5
    def f (self, z, w):
        self.y = w
        print self.x
    def __init__(self, a):
        self.x = a
    y = 6
    q = 7
b = 300
a = A(2)
# a.f(3, 4)
print a.y, a.x
a.f(3,4)
print a.y, a.x
