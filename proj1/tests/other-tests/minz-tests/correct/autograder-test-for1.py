L = [ 3, 7, 11, 5 ]

for i in L: print i

for i in L:
    print i
    print i ** 2

for i in L:
    print i
    print i ** 2
else:
    print -i

x = 4

for i in L:
    if i > x:
        print i
        break

x = 6

for i in L:
    if i > x:
        continue
    print i
    
L = ((1, 2), (3, 4), (5, 6))
for i, j in L:
    print j, i, j+i
for (i, j) in L:
    print j, i, j+i

