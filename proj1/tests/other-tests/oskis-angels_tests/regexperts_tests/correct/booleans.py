# Checks correct boolean values and indentation

if 1:
    if True:
        print 1
if 'fdsa':
    print 's'
if [12] or (1) or {1:1}:
    print 'd'
if '' or [] or () or {} or 0 or False:
    print "Error"
