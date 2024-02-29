
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
[^ \t\n;]*\\[^\n\t]* {
  std::string temp = strdup(yytext + 1);
  temp[yyleng - 2] = '\0';
  std::string str = "\\";
  std::string out = "";
  temp[yyleng - 2] = '\0';
  int ptr = 0;
  size_t find_pos = temp.find(str);
  while (find_pos != std::string::npos) {
    out += temp.substr(ptr, find_pos);
    switch (out[find_pos + 1]) {
      case 'n':
        // do \n
        out += "\n";
        break;
      case 't':
        out += "\t";
        break;
      case 'r':
        out += "\r";
        break;
      default:
        out += out[find_pos + 1];
    }
    ptr = find_pos + 2;
    find_pos = temp.find(str);
  }
  //yylval.cpp_string = temp;
  yylval.cpp_string = new std::string(out);
  return WORD;
}
[^ \t\n;][^ \t\n;]*  {
  /* Assume that file names have only alpha chars */
  yylval.cpp_string = new std::string(yytext);
  return WORD;
}
