#Test that you cannot declare a function in a while loop
def bar(y):
	while (y < 10):
		def foo(x):
			return x
		y = y + 1

bar(3)