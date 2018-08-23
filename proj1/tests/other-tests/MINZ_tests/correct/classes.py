#Classes
class class1:
    def __init__(self):
    	self.truth = True
    def do_stuff(self, end_num):
        for i in range(0, end_num):
        	self.truth = True if i % 2 == 0 else False 
hey = class1()
hey.do_stuff(12)
print(hey.truth) #Expect False 
