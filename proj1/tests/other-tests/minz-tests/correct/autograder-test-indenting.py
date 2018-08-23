# Check that various indentation styles work

if True:
    print 1

if True:
        print 2

if True:
  print 1


if True:
	print 1   # This line starts with a tab
        print 2   # This starts with 8 blanks
   	print 3   # This has both

