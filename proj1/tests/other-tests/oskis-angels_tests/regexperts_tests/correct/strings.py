# This is a comment
# Checks if docstrings are still recognized as strings
# Raw strings and escape symbols are allowed
# Format strings

"""Docstrings are also valid"""

x = """This is a docstring"""
print(x)
print r"\""
print "\""
y = """ ' " ' "" #### fdsa# \
fdsafdsa
\"\"""" + "fdsa"

r""" fdsa r"""

z = "foo" "bar`"
a = "bar" R"ba`r" "fo```o"
b = r"bar" R"bar" 
c = """hi""" """you"""
