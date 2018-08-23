class A(object):
    x = 3
    def f(self):
        if self.x > 0: # OK
            return x # ERROR: x is unknown here.
