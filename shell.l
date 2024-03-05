
/*
 *
 * CS-252 
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{

#include <cstring>
#include <string>
#include <vector>
#include <iostream>
#include "y.tab.hh"

static  void yyunput (int c,char *buf_ptr  );

void myunputc(int c) {
  unput(c);
}

%}

%option noyywrap

%%

"#".*\n {
  /* Discard comments starting with # */
}

\n {
  return NEWLINE;
}

[ \t] {
  /* Discard spaces and tabs */
}

">" {
  return GREAT;
}

"<" {
  return LESS;
}

"|" {
  return PIPE;
}

">>" {
  return GREATGREAT;
}
"2>" {
  return TWOGREAT;
}
">&" {
  return GREATAMPERSAND;
}

">>&" {
  return GREATGREATAMPERSAND;
}

"&" {
  return AMPERSAND;
}

"[" {
  return LBRACKET;
}

"]" {
  return RBRACKET;
}

"if" {
  return IF;
}

"then" {
  return THEN;
}

"fi" {
  return FI;
}

"while" {
  return WHILE;
}

"do" {
  return DO;
}

"done" {
  return DONE;
}

"for" {
  return FOR;
}

"in" {
  return IN;
}

";" {
  return SEMI;
}

\"[^\n\"]*\" {
  char *temp;
  temp = strdup(yytext + 1);
  temp[yyleng - 2] = '\0';
  yylval.cpp_string = new std::string(temp);
  return WORD;
}

[^ |>\t\n;][^ |>\t\n;]*  {
  /* Assume that file names have only alpha chars */
  yylval.cpp_string = new std::string(yytext);
  std::string temp(*yylval.cpp_string);
  for (int i = 0; i < temp.size(); i++) {
    if (temp[i] == '\\') {
      temp.erase(i,1);
    }
  }
  yylval.cpp_string = new std::string(temp);
  return WORD;
}


\$\([^\n]*\) {
  char* temp;
  temp = strdup(yytext + 1);
  temp[yyleng - 2] = '\0';
  yllval.cpp_string = new std::string(temp);
  return WORD:
}
