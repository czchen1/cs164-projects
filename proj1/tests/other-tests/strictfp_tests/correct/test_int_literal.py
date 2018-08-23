# https://github.com/python/cpython/blob/84471935ed2f62b8c5758fd544c7d37076fe0fa5/Lib/test/test_int_literal.py
print 0x0 == 0X0
print 0x1 == 0X1
print 0x123def == 0X123def
print -(0x0) == 0
print -(0x10) == -16
print -0x0 == -16
