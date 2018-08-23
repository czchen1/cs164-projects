def f():
	def g():
		x = y
		# y = 10 # would cause an error
		print(x)
		print(y)
	# g()  # would be an error
	y = 3
	g()  # okay

f()
