keywords = ['as', 'assert', 'del', 'except', 'exec', 'from', 'finally', 'future', 'global', 'import', 'lambda', 'raise', 'try', 'with', 'yield']

for word in keywords:
    with open('id_' + word + '.py', mode='w') as file:
        file.write(word + " = 1")
