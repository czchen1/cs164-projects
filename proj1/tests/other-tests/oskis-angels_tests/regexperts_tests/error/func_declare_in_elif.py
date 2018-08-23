#test that you cannot declare functions in elif statements
def foo(x):
	if x < 5:
		return x
	elif 5 < x < 10:
		def bar(y):
			if y < 10:
				return y
			return 11
		return 7
	return 10

foo(6)