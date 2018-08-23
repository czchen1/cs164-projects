class A of [$T]:
    x::list of [$T] = [] # T is in the class header
    y = 3              # y has type int.
    z = []             # z's type is eventually resolved...
    z[0:0] = [3]       # ... at this point.
