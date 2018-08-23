/* -*- mode: fundamental; c-file-style: "stroustrup"; indent-tabs-mode: nil -*- */

/* apyc-lexer.l: Flex Lexer */

/* Authors: YOUR NAMES HERE */

  #define YY_DECL int yylex_raw() 

%{
#include <vector>

vector<int> indentation_stack;
bool dedenting = false;
bool debug_indent_count = false; 

int indent_dedent (char* yytext) {
/*    Counts the number of spaces. A tab cause the space count*/
    /*to jump to the nearest multiple of 8 above the current count.*/
    /*Turn on debug_indent_count to see the indentation level.*/
    int spaces = 0;
    for (char* i = yytext; *i; i++) {
        if (*i == ' ') {
            if (debug_indent_count) { yyerror("spaces"); }
            spaces++;
        }
        else if (*i == '\t') {
            spaces = (spaces / 8) * 8 + 8;
            if (debug_indent_count) { yyerror("tabs"); }
        }
    }

    if (debug_indent_count) {
        char integer_string[32];
        sprintf(integer_string, "%d", spaces);
        yyerror(integer_string);
    }

    /* Initialize stack. First line must have no indent*/
    if (indentation_stack.size() == 0) {
        if (spaces == 0)
            indentation_stack.push_back(spaces);
        else
            yyerror("Bad indentation");
        return 0;
    }
    /* Indent */
    int top = indentation_stack.back();
    if (!dedenting && top < spaces) {
        indentation_stack.push_back(spaces);
        return INDENT;
    }
    /* Same indent level as above line*/
    else if (top == spaces) return 0;
    /* Dedent */
    else if (top > spaces) {
        indentation_stack.pop_back();
        dedenting = true;
        return DEDENT;
    }
    /* Error if dedent level is incorrect.*/
    yyerror("Bad indentation");
    return 0;
}

/* Keeps track of if we are within (), [], {} (outside of a string)
In this case, \n are treated as spaces. */
int open_paren;
int open_sqr;
int open_curly;

bool within_paren (char yytext) {

    if (yytext == '(') {
        open_paren ++;
    }
    else if (yytext == '[') {
        open_sqr ++;
    }
    else if (yytext == '{') {
        open_curly ++;
    }
    else if (yytext == ')') {
        open_paren --;
    }
    else if (yytext == ']') {
        open_sqr --;
    }
    else if (yytext == '}') {
        open_curly --;
    }
    else { yyerror("Shouldn't be here");}

    if (open_paren + open_sqr + open_curly > 0)
        return true;
    else
        return false;
}

%}

%option yylineno noyywrap

ALPHA [a-zA-Z_]
ALNUM [a-zA-Z_0-9]

%x MIDLINE  IN_PAREN

%%

  /* The following are incomplete.  Complete them appropriately.  */

  /* Normally, throw away whitespace other than newline. */
                        /* Line with nothing but whitespace. Moves to next line, remains in INITIAL state. */
<INITIAL>[\t ]*\n       {      }
                        /* Line with nothing but whitespace + comment. Moves to next line, remains in INITIAL state. */
<INITIAL>[ \t\r]*#.*\n  {      }
                        /* Remove all other comments. Do not remove the \n after the comment. */
<*>#.*                  {      }
                        /* Start of line with no indent (whitespace). Line does not begin with comment. */
<INITIAL>[^ #\t\r\n]+   {yyless(0); int i = indent_dedent((char*)"");
                            if (i != DEDENT) { BEGIN(MIDLINE); dedenting = false; }
                            else {
                                return i; 
                            }
                        } 
                        /* Start of line indented with whitespace, not a comment */
<INITIAL>[ \t\r]*/[^ #\t\r\n]  {  int i = indent_dedent(yytext);
                           if (i!=0) {
                               if (i == DEDENT) {yyless(0);}
                               else {
                                   BEGIN(MIDLINE);
                                   dedenting = false;
                                }
                               return i;
                            }
                            BEGIN(MIDLINE);
                            dedenting = false;
                        }

                        /* Midline, throw away whitespace */
<MIDLINE>[ \t\r]+       {      }
                        /* When we see a newline: 1. Line carriage--"continue" the line,
                        2. MIDLINE--move to INITIAL state, stop dedenting??
                        3. In paren-- get rid of it */
<MIDLINE,IN_PAREN>\\[ \t\r]*\n   { /* Line Carriage */ }
<MIDLINE>\n     { dedenting = false; BEGIN(INITIAL); return NEWLINE; }
<IN_PAREN>[ \t\r\n]*    {       }

                        /* */
<IN_PAREN>\"([^\n"\\]|\\.)* { yyerror("unclosed string"); }
<IN_PAREN>\'([^\n'\\]|\\.)* { yyerror("unclosed string"); }
 
<MIDLINE,IN_PAREN>{
(0[bB][01]+|0[0-7]*|[1-9][0-9]*|0[xX][0-9A-Fa-f]+)       {  /* FIXME */
          yylval.str = new string (yytext);
          return INT;
    }



  /* Keywords */
print             { return PRINT; }
False             { return FALSE; }
True              { return TRUE; }
None              { return NONE; }
and               { return AND; }
or                { return OR; }
not               { return NOT; }
else              { return ELSE; }
if                { return IF; }
elif              { return ELIF; }
in                { return IN; }
"not in"          { return NOT_IN; }
is                { return IS; }
"is not"          { return IS_NOT; }
break             { return BREAK; }
continue          { return CONTINUE; }
return            { return RETURN; }
while             { return WHILE; }
for               { return FOR; }
native            { return NATIVE; }
of                { return OF; }
"::"              { return TYPEASSIGN; }
class             { return CLASS; }
def               { return DEF; }
pass              { return PASS; }
"->"              { return ARROW; }

as                { yyerror("invalid identifier"); }
assert            { yyerror("invalid identifier"); }
del               { yyerror("invalid identifier"); }
except            { yyerror("invalid identifier"); }
exec              { yyerror("invalid identifier"); }
from              { yyerror("invalid identifier"); }
finally           { yyerror("invalid identifier"); }
future            { yyerror("invalid identifier"); }
global            { yyerror("invalid identifier"); }
import            { yyerror("invalid identifier"); }
lambda            { yyerror("invalid identifier"); }
raise             { yyerror("invalid identifier"); }
try               { yyerror("invalid identifier"); }
with              { yyerror("invalid identifier"); }
yield             { yyerror("invalid identifier"); }

\$[a-zA-Z_][a-zA-Z_0-9]* { yylval.str = new string (yytext); return TYPEVAR;}
[a-zA-Z_][a-zA-Z_0-9]*   { yylval.str = new string (yytext); return ID;}

  /* Punctuation */
  /* By convention, for one-character tokens written in the grammar as
   * character constants (such as ','), Bison uses the character value itself
   * (as an integer). */
[,;:.]                    { return yytext[0]; }
[()\[\]\{\}]              { if(within_paren(yytext[0])) BEGIN(IN_PAREN); else BEGIN(MIDLINE); return yytext[0]; }
\*\*                      { return POW; }
\<=                       { return LEQ; }
>=                        { return GEQ; }
==                        { return EQ_OP; }
!=                        { return NEQ; }              
[\+\-\*%/\<>=]            { return yytext[0]; }

  /* Strings */
(\"([^\n"\\]|\\(.|\n))*\")|('([^\n'\\]|\\(.|\n))*')             { yylval.str = new string (yytext); return STRING; }

((r|R)\"([^\n"\\]|\\(.|\n))*\")|((r|R)'([^\n'\\]|\\(.|\n))*')   { yylval.str = new string (yytext); return STRING; }


(r|R)?\"\"\"(\"{0,2}([^"\\]|\\(.|\n)))*\"\"\"         { yylval.str = new string (yytext); return STRING; }

(r|R)?'''('{0,2}([^'\\]|\\(.|\n)))*\'''               { yylval.str = new string (yytext); return STRING; }

.			{yyerror("Bad char");}
}
%%

static vector<string> sourceFileNames;
static vector<int> firstLines;

bool
initLexer (const string& fileName)
{
    FILE* input = fopen (fileName.c_str (), "r");
    if (input == NULL) {
        return false;
    }

    firstLines.push_back (yylineno);
    sourceFileNames.push_back (fileName);
    yyin = input;
    // FIXME: Other stuff here?
    return true;
}

int
yylex ()
{
    int result = yylex_raw();
    yylloc = yylineno;
    if (result == 0 && indentation_stack.size() > 1) {
        indentation_stack.pop_back();
        return DEDENT;
    }
    return result;
}

string
currentFileName ()
{
    return sourceFileNames.back ();
}

Location
currentLocation ()
{
    return yylineno;
}

string
locationFileName (Location loc)
{
    if (loc == 0) {
        return "?";
    } else {
        for (size_t i = 1; i < sourceFileNames.size(); i += 1) {
             if (firstLines[i] > loc) {
                 return sourceFileNames[i - 1];
             }
         }
         return sourceFileNames.back ();
    }
}

Location
locationLine (Location loc)
{
    if (loc == 0) {
        return 0;
    } else {
        for (size_t i = 1; i < sourceFileNames.size(); i += 1) {
             if (firstLines[i] > loc) {
                 return loc - firstLines[i - 1] + 1;
             }
         }
         return loc - firstLines.back () + 1;
    }
}
