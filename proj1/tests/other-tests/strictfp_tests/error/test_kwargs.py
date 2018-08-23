def f(**kwargs):
    print kwargs

f(hello=1, world=2)
f(**{'hello': 1, 'world': 2})
