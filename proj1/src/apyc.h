/* -*- mode: C++; c-file-style: "stroustrup"; indent-tabs-mode: nil; -*- */

/* Interfaces for apyc. */

#ifndef _APYC_H_
#define _APYC_H_

#include <cstdio>
#include <string>
#include "ast.h"

/** Print an error message in standard Unix format:
 *      <filename>:<linenum>: <message>
 *  where <filename> and <linenum> denote the file and line number
 *  containing source location LOC, and <message> is composed from
 *  FORMAT and the trailing arguments as
 *  for the printf family. */
extern void error (Location loc, const char* format, ...);
/** Print an error message (without file or line number indications)
 *  composed from FORMAT and the trailing arguments as for the printf
 *  family. */
extern void error_no_file (const char* format, ...);

/** Return a value that encodes the current line number and location
 *  in the source files. */
extern Location currentLocation ();

/** Given an encoded source location (as produced by currentLocation),
 *  return the name of the source file containing that location. */
extern std::string locationFileName (Location loc);

/** Given an encoded source location (as produced by currentLocation),
 *  return the line number in the source file containing that location. */
extern Location locationLine (Location loc);

/** (Re-) initialize the lexer to start reading from source file
 *  FILENAME.   Returns true on success, false if FILENAME can't be
 *  read. */
extern bool initLexer (const std::string& fileName);

extern void parse (FILE*, const std::string&);

extern bool debugParser;

#endif

