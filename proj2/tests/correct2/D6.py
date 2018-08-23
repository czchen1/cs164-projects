def g():
    def h():
        print y
    y = 3
    h()
g()

