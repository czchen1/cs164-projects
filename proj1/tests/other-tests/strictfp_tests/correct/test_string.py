# https://github.com/python/cpython/blob/84471935ed2f62b8c5758fd544c7d37076fe0fa5/Lib/test/test_grammar.py
x = ''; y = ""
print x == y
x = '\''; y = "'"
print x == y
x = '"'; y = "\""
print x == y
x = "doesn't \"shrink\" does it"
y = 'doesn\'t "shrink" does it'
print x == y
x = "does \"shrink\" doesn't it"
y = 'does "shrink" doesn\'t it'
print x == y
x = """
The "quick"
brown fox
jumps over
the 'lazy' dog.
"""
y = '\nThe "quick"\nbrown fox\njumps over\nthe \'lazy\' dog.\n'
print x == y
y = '''
The "quick"
brown fox
jumps over
the 'lazy' dog.
'''
print x == y
y = "\n\
The \"quick\"\n\
brown fox\n\
jumps over\n\
the 'lazy' dog.\n\
"
print x == y
y = '\n\
The \"quick\"\n\
brown fox\n\
jumps over\n\
the \'lazy\' dog.\n\
'
print x == y
