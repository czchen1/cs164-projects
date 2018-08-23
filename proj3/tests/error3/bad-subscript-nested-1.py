def f():
	t = [1, 2, 3]
	def g():
		t[3] = 0
		return 1
	return g
y = f()
print y()
