class A(object):
    def foo(self,x):
        print x

    @classmethod
    def class_foo(cls,x):
        print x

    @staticmethod
    def static_foo(x):
        print x

a=A()
a.class_foo(1)
a.static_foo(1)