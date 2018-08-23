# https://github.com/python/cpython/blob/84471935ed2f62b8c5758fd544c7d37076fe0fa5/Lib/test/test_grammar.py
x = 1; pass
def foo():
    x = 1; pass; return x
print foo()
