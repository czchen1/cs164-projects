for x in 1, 2, 3, 4,: print x

for x in range(5):
    for y in range(5):
        if x + y == 4:
            print "Yes"
else:
    print "No"

def f():
    def g():
        for (x,) in ((2,), (4,), (6,)):
            i = 0
            while i < x:
                print i
                i = i + 1
    g()
f()
