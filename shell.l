
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

[^ \t\n;][^ \t\n;]*  {
  /* Assume that file names have only alpha chars */
  yylval.cpp_string = new std::string(yytext);
  return WORD;
}


[^ \t\n;]*\\[^\n\t]* {
  std::string temp;
  std::string cleaned;
  temp = std::string(yytext, yytext + yyleng);
  temp[yyleng - 1] = '\0';
  for (size_t i = 0; i < temp.length(); i++) {
    /*if (temp[i] == '\\' && i + 1 < temp.length()) {
      i++;
    } else {
      cleaned += temp[i];
    }*/
    if (temp[i] == '\\') {
      size_t start = i;
      while (i + 1 < temp.length && temp[i+1] == '\\') {
        i++;
      }
      size_t count = i - start + 1;
      if (count % 2 == 0) {
        cleaned += std::string(count / 2, '\\');
      } else {
        if (i + 1 < temp.length()) {
          cleaned += temp[i+1];
          i++;
        } else {
          cleaned += '\\';
        }
      }
    } else  {
      cleaned += temp[i];
    }
  }
  yylval.cpp_string = new std::string(cleaned);
  return WORD;
}
