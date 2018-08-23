#Test that we cannot use False as a variable name
def foo(x):
	False = x

foo(5)