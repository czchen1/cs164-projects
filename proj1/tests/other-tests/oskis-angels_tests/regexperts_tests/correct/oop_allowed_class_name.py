#Test that class names with default parent classes are allowed
class Cat:
	weight = 6

	def __init__(self, weight):
		self.weight = weight

	def make_noise(self):
		return "meow"

c = Cat(4)
print(c.make_noise())
