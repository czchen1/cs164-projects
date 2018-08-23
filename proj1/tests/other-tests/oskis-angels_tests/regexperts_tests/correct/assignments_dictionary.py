a = {1 : 2, 'a' : 'b', 'c' : 3}
print a
print a[1]
print a['a']
print a['c']
a[1] = 5
print a[1]
a['key'] = 'val'
print a['key']
b = [1]
a['list'] = b
print a['list']
b[0] = 2
print a['list']
a['a'] = 'd'
print a['a']
a[('a', 'b')] = 'tuple'
print a[('a', 'b')]
