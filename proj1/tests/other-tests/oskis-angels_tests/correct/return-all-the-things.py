def single(x):
    return x

def double(x, y):
    return x, y

def single_enc(x):
    return (x)

def double_enc(x, y):
    return (x, y)

def ret_none(x, y, z):
    return

def ret_literal():
    return 10

def ret_list():
    return [10, 20, 30]

print single(10)
print double(20, 30)
print single_enc(40)
print double_enc(50, 60)
print ret_none(70, 80, 90)
print ret_literal()
print ret_list()
