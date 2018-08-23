def foo(x):
	count = 0
	while (x > 0):
		count = count + 1
		x /= 2
	return count

foo(64)