THE FILES HERE ARE INTENDED TO BE HELPFUL, NOT TO LIMIT WHAT YOU CAN DO.
***IGNORE THEM ENTIRELY, IF YOU PREFER!***  However, as indicated in the handout,
whatever you turn in must satisfy the following criteria:

1. 'make' must compile your program from scratch, producing an executable
   file 'apyc' in this directory.
2. 'make check' must run all your tests against your ./apyc program.
3. 'make APYC=PROG check' must run all your tests against the program
   PROG.
4. The command
      ./apyc --phase=1 FOO1.py
   must produce AST file FOO1.ast.
5. The command
      ./apyc --phase=1 -o BAR FOO1.py
   must produce AST file BAR.
6. Your program must exit with code 1 if there are any errors in any of the
   files, or if there are other errors (such as missing, unreadable, 
   or unwritable files).  It must produce a proper error message for syntax
   errors.
7. Your program must exit with code 0 otherwise.
8. We have also added a -dp option, which turns on parser debugging
   output when present.  If you use something other than Bison, you
   don't need to worry about it.

For the purposes of this first project assignment, the suggested
skeleton is admittedly object-oriented overkill.  It isn't REALLY
necessary to create a full-fledged AST with different node types for
different types of expressions and statements to fulfill the specs of
this project---which is just to print an external version of the AST.
However, this structure will be useful in later phases, when the
differences between types of nodes become more important.

Don't let anything in your finished program remain mysterious to you.
Try to understand everything that's here, and if you can't, then feel
free to ask us what's going on.

We judge your program by how it runs ON THE INSTRUCTIONAL SYSTEMS.  It's 
no good protesting that it works at home; your "customer" is not running on 
your home setup!

Be sure to submit your program properly by tagging the desired commit
as proj1-N for integer numeral N and pushing that commit and tag to
your team's (NOT your personal) repository.  I further suggest that
you check out the tag that you submit into another repository cloned
from your team repository and make sure everything is as you intend (and
that you are not accidentally missing files or even missing the tag
entirely).

The current contents of the files here are extremely skeletal.  Don't expect
that just because you see a class in the skeleton, it is complete.  
Furthermore, you are free to change or discard ANY PART of it as long
as your final program works according to spec.

Likewise, the tests are completely inadequate.

If you add a new source file, be sure to modify Makefile appropriately,
and 'svn add' it as well.  If you remove a file, modify Makefile and 
'svn remove' the old file.  Whenever you add or remove a file, or add, 
remove or modify an #include statement, be sure to run 'make depend'.
This keeps the Makefile informed of what depends on what.

MANIFEST:

   README:     This file.

   Makefile:   General directions for building and testing the program and
               cleaning up.  This mostly delegates to subdirectories src and
               tests.

   Directory src/: 
        apyc.cc:    Contains the main program and some general utilities.
        apyc.h:     Some high-level interfaces between modules.
        ast.h:      Base classes for AST nodes.
        ast.cc:     Method definitions for base classes and
                    definitions for a few widely used AST subtypes.
        exprs.cc:   AST subclass definitions for expression-related AST nodes.
        stmts.cc:   AST subclass definitions for statement-related AST nodes.

        apyc-lexer.l:  A Flex file defining token syntax.
        apyc-parser.y: A Bison grammar file.

        Makefile:   General directions for building the program.

    Directory tests/:
        test-proj1:  A testing script for project 1.  It
                     runs the compiler, unparses the resulting AST,
                     runs Python on the result, and checks the output.

        Makefile:   General directions for testing the program.

        Directory correct/:  Tests that are supposed to be correct programs.

                trivial.py, trivial.in, trivial.std:
                     A sample program, input file, and expected output file.

        Directory error/:    Tests that are supposed to cause errors.

                triviallybad.py:
                     A sample erroneous program.
