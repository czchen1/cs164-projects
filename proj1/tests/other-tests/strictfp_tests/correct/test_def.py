# https://github.com/python/cpython/blob/84471935ed2f62b8c5758fd544c7d37076fe0fa5/Lib/test/test_grammar.py
def f1(): pass
def f2(one_argument): pass
def f3(two, arguments): pass
f1()
f2(1)
f2(1,)
f3(1, 2)
f3(1, 2,)
