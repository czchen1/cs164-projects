# https://github.com/python/cpython/blob/84471935ed2f62b8c5758fd544c7d37076fe0fa5/Lib/test/test_grammar.py
x = 1 \
        + 1
print x
print x \
        + x
# Backslash does not mean continuation in comments :\
x = 0
print x
