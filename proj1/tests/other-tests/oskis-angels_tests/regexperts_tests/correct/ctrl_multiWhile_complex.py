tupl = (1, 2, 3, 4, 5, 6)
i = 0
while i < len(tupl):
    if i == 4:
        print "Found 4"
    elif i == 5:
        print "Found 5"
    else:
        j = 0
        while j < len(tupl):
            print tupl[j]
            j = j + 1
    i = i - -1;
else:
    print "hi"
