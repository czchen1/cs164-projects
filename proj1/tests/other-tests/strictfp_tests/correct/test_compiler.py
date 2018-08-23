# https://github.com/python/cpython/blob/84471935ed2f62b8c5758fd544c7d37076fe0fa5/Lib/test/test_compiler.py

class Toto:
    """docstring"""
    pass

a, b = 2, 3
print a, b
[c, d] = 5, 6
print c, d
d = {'a': 2}
print d
d = {}
print d
t = ()
print t
t = (1, 2)
print t
l = []
print l
l = [1, 2]
print l
if l:
    pass
else:
    a, b = b, a
print a, b
