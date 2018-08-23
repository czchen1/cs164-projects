def f():
	x = 4
	def g():
		def g1(x, y, z):
			x = 4; y = 3; z = 2
		return 5
	g()


def h():
	def i():
		print "hi"

def z(): print "one line"; print "one line"
