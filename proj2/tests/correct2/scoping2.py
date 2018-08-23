def f():
	def g():
		x = 40
		print 'and:'
		print h
	i = 0
	while i < 3:
		print i
		h = i * i
		i += 1
	g()  # okay
	# print x  # would cause error!!

f()
