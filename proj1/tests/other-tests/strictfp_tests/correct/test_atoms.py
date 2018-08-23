# https://github.com/python/cpython/blob/84471935ed2f62b8c5758fd544c7d37076fe0fa5/Lib/test/test_grammar.py
x = (1)
x = (1 or 2 or 3)
x = (1 or 2 or 3, 2, 3)
print x

x = []
x = [1]
x = [1 or 2 or 3]
x = [1 or 2 or 3, 2, 3]
print x
x = []
print x

x = {}
x = {'one': 1}
x = {'one': 1,}
print x['one']
x = {'one' or 'two': 1 or 2}
print 'one' in x
print 'two' in x
print x['one']
x = {'one': 1, 'two': 2}
x = {'one': 1, 'two': 2,}
x = {'one': 1, 'two': 2, 'three': 3, 'four': 4, 'five': 5, 'six': 6}

x = x
x = 'x'
x = 123
