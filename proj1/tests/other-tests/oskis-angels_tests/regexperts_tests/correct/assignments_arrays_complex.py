a = [[1,2], [3,4], [5,6]]
print a
print a[0]
b = [['1'], ['2'], '3']
print b
print b[0]
b[0] = a[2]
print b[0]
print b
print a[0][0]
a[0][0] = b
print a
a[0][0] = b[0][0]
print a
(a[0], b[0], c) = a[0], b[0] + [1], a[0][0]
print a[0], b[0], c
[a[0], b[0], c] = 1, 2, a[0][0]
print a[0], b[0], c
