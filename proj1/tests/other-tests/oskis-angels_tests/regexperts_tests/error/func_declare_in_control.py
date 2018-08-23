#test that you cannot declare functions in if statements
def foo(x):
	if x < 5:
		def bar(y):
			if y < 10:
				return y
			return 11
		return x
	return 10

foo(3)