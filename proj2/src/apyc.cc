/* -*- mode: C++; c-file-style: "stroustrup"; indent-tabs-mode: nil; -*- */

/* apyc: A PYthon Compiler. */

/* Authors:  YOUR NAMES HERE */

#include <cstdio>
#include <cstdlib>
#include <string>
#include <cstring>
#include <cstdarg>
#include <libgen.h>
#include <iostream>
#include "apyc.h"

using namespace std;

/** Cumulative error count. */
int errCount;
/** True when error messages are to be printed.  Turned off to allow
 *  overloading resolution without messages. */
static bool reportErrors;

static string file_name;

static void
error1 (Location loc, const char* format, va_list ap)
{
    if (reportErrors) {
        if (loc != -1) {
            fprintf (stderr, "%s:%ld: ",
                     locationFileName (loc).c_str (), locationLine (loc));
        }
        vfprintf (stderr, format, ap);
        fprintf (stderr, "\n");
    }
    va_end (ap);
    errCount += 1;
}

void
fatal (const char* format, ...)
{
    va_list ap;
    reportErrors = true;
    va_start (ap, format);
    error1 (-1, format, ap);
    exit (1);
}

void
error (Location loc, const char* format, ...)
{
    va_list ap;
    va_start (ap, format);
    error1 (loc, format, ap);
}

void
error (AST_Ptr place, const char* format, ...)
{
    va_list ap;
    va_start (ap, format);
    error1 (place->location (), format, ap);
}

void
error_no_file (const char* format, ...) 
{
    va_list ap;
    va_start (ap, format);
    error1 (-1, format, ap);
}

int
numErrors ()
{
    return errCount;
}

int
setNumErrors (int num)
{
    int count0 = errCount;
    errCount = num;
    return count0;
}

bool
setErrorReporting (bool on)
{
    bool reportErrors0 = reportErrors;
    reportErrors = on;
    return reportErrors0;
}

static void
Usage ()
{
    fatal ("Usage: apyc --phase=(1|2) [ -dp ] [ -o OUTFILE ] DIR/BASE.py\n");
    exit (1);
}

static AST_Ptr
phase1 (const string& inFileName)
{
    FILE* inFile = fopen (inFileName.c_str (), "r");
    if (inFile == NULL) {
        error_no_file ("Could not open %s", inFileName.c_str ());
        exit (1);
    }
    AST_Ptr result = parse (inFile, inFileName);
    fclose (inFile);
    return result;
}

static AST_Ptr
phase2 (AST_Ptr tree)
{
    return tree->doOuterSemantics ();
}


static void
setOutputFile(const char* fileName, string& baseName, const char* ext)
{
    string outFileName;
    if (fileName == nullptr) {
        outFileName = baseName + ext;
    } else {
        outFileName = fileName;
    }

    if (freopen (outFileName.c_str (), "w", stdout) == nullptr) {
        error_no_file ("could not write to %s", outFileName.c_str ());
        exit (1);
    }
}

static void
post2_call_helper (AST_Ptr tree, AST_Ptr firstChild, int i) {
    AST_Vect kids;
    
    if (firstChild->child(0)->typeIndex() == ID_AST) {
        kids = firstChild->children();
        if (firstChild->child(0)->text().compare("__setitem__") == 0) {
            kids.pop_back();
            kids.push_back(NODE (NEXT_VALUE_AST));
            tree->setChild(i, NODE (SUBSCRIPT_ASSIGN_AST, kids));
        } else if (firstChild->child(0)->text().compare("__setslice__") == 0) {
            kids.pop_back();
            kids.push_back(NODE (NEXT_VALUE_AST));
            tree->setChild(i, NODE (SLICE_ASSIGN_AST, kids));
        }
    }
}

static AST_Ptr
post2 (AST_Ptr tree) {
    if (tree == NULL) {
        return NULL;
    }
    
    AST_Ptr firstChild, targetListChild;
    switch (tree->typeIndex()) {
        case ASSIGN_AST:
            firstChild = tree->child(0);
            if (firstChild->typeIndex() == TARGET_LIST_AST) {
                for (int i = 0; i < firstChild->arity(); ++i) {
                    targetListChild = firstChild->child(i);
                    if (targetListChild->typeIndex() == CALL_AST) {
                        post2_call_helper(firstChild, targetListChild, i);
                    }
                }
            } else if (firstChild->typeIndex() == CALL_AST) {
                post2_call_helper(tree, firstChild, 0);
            }
        default:
            for (auto& c : *tree) {
                c = post2(c);
            }
            return tree;
    }
}

int
main (int argc, char* argv[])
{
    int i;
    int phase;
    char* outFileName;
    string preludeName;

    phase = -1;
    outFileName = nullptr;
    preludeName =
        string(dirname(realpath(argv[0], nullptr))) + "/lib/prelude.py";
    setErrorReporting (true);

    for (i = 1; i < argc; i += 1) {
        string opt = argv[i];
        if (opt == "-o" && i < argc-1) {
            outFileName = argv[i+1];
            i += 1;
        } else if (opt.compare (0, 8, "--phase=") == 0)
            phase = atoi(opt.c_str () + 8);
        else if (opt == "-dp") 
            debugParser = true;
        else if (opt.size () == 0 || opt[0] == '-')
            Usage();
        else
            break;
    }
    errCount = 0;

    if (i != argc - 1 || phase < 1 || phase > 2)
        Usage ();

    string infileName = argv[i];
    if (infileName.size () < 4 
        || infileName.compare (infileName.size ()-3, 3, ".py") != 0) {
        error_no_file ("Unknown file type: %s\n", argv[i]);
        exit (1);
    }
    string baseFileName = string (infileName, 0, infileName.size () - 3);

    switch (phase) {
    default:
        Usage ();

    case 1: {
        AST_Ptr tree = phase1 (infileName);
        if (tree != nullptr) {
            setOutputFile (outFileName, baseFileName, ".ast");
            tree->print (cout, 0);
            cout << endl;
        }
        break;
    }

    case 2: {
        AST_Ptr prog = phase1 (infileName);
        AST_Ptr prelude = phase1(preludeName);
        
        if (errCount > 0)
            break;
        
        AST_Ptr tree = post1 (prog, prelude);
        tree = phase2 (tree);
        tree = post2 (tree);
        if (errCount > 0)
            break;

        setOutputFile (outFileName, baseFileName, ".dast");
        tree->print (cout, 0);
        cout << endl;
        //Decl_Set decls =  tree->reachableDecls ();
        //cout << "Number of reachable decls %i", decls.size();
        //cout << endl;
        outputDecls (cout, tree->reachableDecls ());
    }
    }

    exit (errCount == 0 ? 0 : 1);
}
