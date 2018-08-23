#Test that we cannot use None as a variable name
def foo(x):
	None = 5

foo(5)