# https://github.com/python/cpython/blob/84471935ed2f62b8c5758fd544c7d37076fe0fa5/Lib/test/test_grammar.py
a = '01234'
s = a[0]
print s
s = a[-1]
print s
s = a[0:5]
print s
s = a[:5]
print s
s = a[0:]
print s
s = a[:]
print s
s = a[-5:]
print s
s = a[:-1]
print s
s = a[-4:-3]
print s

d = {}
d[1] = 1
d[1,] = 2
d[1,2] = 3
d[1,2,3] = 4
print d[1], d[1,], d[1,2], d[1,2,3]
