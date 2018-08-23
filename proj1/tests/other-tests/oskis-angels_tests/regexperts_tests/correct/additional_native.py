# Test using native functions.
def my_print_fn(my_str):  native "printf"

def regular_fn():
    print "I am not native"

def open_a_file(filename, mode):
    native "fopen"
