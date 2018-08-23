class MyMap of [$Key, $Value]:

    def __init__(self):
        mmap :: dict of [$Key, $Value] = dict of [$Key, $Value]()
        self.map = mmap
    
    def get(self, key :: $Key) :: $Value:
        return self.map[key]
    
    def set(self, key :: $Key, value :: $Value):
        self.map[key] = value

x :: list of int = [1, 2, 3, 4]
print(x)

x :: MyMap of [int, int] = MyMap()
x.set(3, 4)
print(x.get(3))
