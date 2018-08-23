# This is a small test program (line numbers to left)

def f(n):
   i::int = 0
   while i <= n:
     if 1 < i % 7 <= 2:
        print i,
   else:
     s = i + 2; t = t + s ** 2
     print "s =", s, "t =", t
