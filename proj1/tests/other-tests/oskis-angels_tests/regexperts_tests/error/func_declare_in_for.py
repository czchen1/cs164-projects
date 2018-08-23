#Test that you cannot declare a function in a for loop
def bar(x):
	for i in range(x):
		def foo(y):
			return y
		foo(i)
	return x

bar(10)