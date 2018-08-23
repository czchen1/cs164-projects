# https://github.com/python/cpython/blob/84471935ed2f62b8c5758fd544c7d37076fe0fa5/Lib/test/test_dict.py
print dict() == {}
print dict() is {}
print not {} is True
print {1: 2}
print bool({})
print bool({1: 2})
d = {'a': 1, 'b': 2}
print 'a' in d
print 'b' in d
print 'c' in d
print d['a']
print d['b']
