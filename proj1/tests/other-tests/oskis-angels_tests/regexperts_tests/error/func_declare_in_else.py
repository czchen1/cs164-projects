#test that you cannot declare functions in else statements
def foo(x):
	if x < 5:
		return x
	elif 5 < x < 10:
		return 10
	else:
		def bar(y):
			if y < 10:
				return y
			return 11
		return 7

foo(12)