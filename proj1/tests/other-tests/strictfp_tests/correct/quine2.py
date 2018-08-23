# https://stackoverflow.com/questions/33071521/how-does-this-python-3-quine-work
s='s=%r;print(s%%s)';print(s%s)
