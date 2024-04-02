
/*
 * CS-252
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%code requires 
{
#include <string>
#include <stack>

#if __cplusplus > 199711L
#define register      // Deprecated in C++11 so remove the keyword
#endif
}

%union
{
  char        *string_val;
  // Example of using a c++ type in yacc
  std::string *cpp_string;
}

%token <cpp_string> WORD WORD2
%token NOTOKEN GREAT GREATGREAT GREATAMPERSAND GREATGREATAMPERSAND 
%token AMPERSAND PIPE LESS NEWLINE IF FI THEN LBRACKET RBRACKET SEMI
%token DO DONE WHILE FOR IN TWOGREAT

%{
//#define yylex yylex
#include <cstdio>
#include <stack>
#include "Shell.hh"




void yyerror(const char * s);
int yylex();

%}

%%

goal: command_list;

arg_list:
        arg_list WORD { 
          Shell::TheShell->_simpleCommand->insertArgument( $2 ); }
        | /*empty string*/
	;

cmd_and_args:
  	WORD { 
          Shell::TheShell->_simpleCommand = new SimpleCommand(); 
          Shell::TheShell->_simpleCommand->insertArgument( $1 );
        } 
        arg_list
	;

pipe_list:
        cmd_and_args 
	    { 
		Shell::TheShell->_pipeCommand->insertSimpleCommand( 
		    Shell::TheShell->_simpleCommand ); 
		Shell::TheShell->_simpleCommand = new SimpleCommand();
	    }
	| pipe_list PIPE cmd_and_args 
	    { 
		Shell::TheShell->_pipeCommand->insertSimpleCommand( 
		    Shell::TheShell->_simpleCommand ); 
		Shell::TheShell->_simpleCommand = new SimpleCommand();
	    }
	;

io_modifier:
	   GREATGREAT WORD {
      if(Shell::TheShell->_pipeCommand->_outFile != NULL) {
        fprintf(stderr, "Ambiguous output redirect.\n");
      }
		Shell::TheShell->_pipeCommand->_outFile = $2;
    Shell::TheShell->_pipeCommand->append_out = true;
     }
	 | GREAT WORD 
	    {
      if(Shell::TheShell->_pipeCommand->_outFile != NULL) {
        fprintf(stderr, "Ambiguous output redirect.\n");
      }
		Shell::TheShell->_pipeCommand->_outFile = $2;
    Shell::TheShell->_pipeCommand->append_out = false;
    }
	 | GREATGREATAMPERSAND WORD {
      if(Shell::TheShell->_pipeCommand->_outFile != NULL) {
        fprintf(stderr, "Ambiguous output redirect.\n");
      }
    Shell::TheShell->_pipeCommand->_outFile = $2;
    Shell::TheShell->_pipeCommand->_errFile = $2;
    Shell::TheShell->_pipeCommand->append_out = true;
    Shell::TheShell->_pipeCommand->append_err = true;
   }
	 | GREATAMPERSAND WORD {
      if(Shell::TheShell->_pipeCommand->_outFile != NULL) {
        fprintf(stderr, "Ambiguous output redirect.\n");
      }
    Shell::TheShell->_pipeCommand->_outFile = $2;
    Shell::TheShell->_pipeCommand->_errFile = $2;
   }
	 | LESS WORD {
    Shell::TheShell->_pipeCommand->_inFile = $2;
   }
   | TWOGREAT WORD {
    Shell::TheShell->_pipeCommand->_errFile = $2;
   }

	;

io_modifier_list:
	io_modifier_list io_modifier
	| /*empty*/
	;

background_optional: 
	AMPERSAND {
    Shell::TheShell->_pipeCommand->_background = true;
  }
	| /*empty*/ {
  Shell::TheShell->_pipeCommand->_background = false;
  }
	;

SEPARATOR:
	NEWLINE
	| SEMI
	;

command_line:
	 pipe_list io_modifier_list background_optional SEPARATOR 
         { 
         //Shell::TheShell->listCommandStack.top()->insertCommand(Shell::TheShell->_pipeCommand);
	   Shell::TheShell->_listCommands->
		insertCommand(Shell::TheShell->_pipeCommand);
	    Shell::TheShell->_pipeCommand = new PipeCommand(); 
         }
        | if_command SEPARATOR 
         {
            Shell::TheShell->_listCommands->
            insertCommand(Shell::TheShell->_ifCommand);
         }
        | while_command SEPARATOR {
            //Shell::TheShell->_listCommands->
            //insertCommand(Shell::TheShell->_ifCommand);
            //IfCommand* completedIfCommand = Shell::TheShell->ifCommandStack.top();

            //ListCommands* completedListCommands = Shell::TheShell->listCommandStack.top();

            //completedListCommands->insertCommand(completedIfCommand);
            Shell::TheShell->listCommandStack.top()->insertCommand(Shell::TheShell->ifCommandStack.top());
            Shell::TheShell->ifCommandStack.pop();
            //Shell::TheShell->ifCommandStack.pop();
            //Shell::TheShell->listCommandStack.pop();
        }
        | for_command SEPARATOR {printf("for\n"); }
        | SEPARATOR /*accept empty cmd line*/
        | error SEPARATOR {yyerrok; Shell::TheShell->clear(); }
	;          /*error recovery*/

command_list :
     command_line 
	{ 
	   Shell::TheShell->execute();
	}
     | 
     command_list command_line 
	{
	    Shell::TheShell->execute();
	}
     ;  /* command loop*/

if_command:
    IF LBRACKET 
	{ 
	    Shell::TheShell->_level++; 
	    Shell::TheShell->_ifCommand = new IfCommand();
	} 
    arg_list RBRACKET SEMI THEN 
	{
	    Shell::TheShell->_ifCommand->insertCondition( 
		    Shell::TheShell->_simpleCommand);
	    Shell::TheShell->_simpleCommand = new SimpleCommand();
	}
    command_list FI 
	{ 
	    Shell::TheShell->_level--; 
	    Shell::TheShell->_ifCommand->insertListCommands( 
		    Shell::TheShell->_listCommands);
	    Shell::TheShell->_listCommands = new ListCommands();
	}
    ;

while_command:
    WHILE LBRACKET {
      Shell::TheShell->_level++;
      Shell::TheShell->listCommandStack.push(new ListCommands());
      Shell::TheShell->ifCommandStack.push(new IfCommand());
	    //Shell::TheShell->_ifCommand = Shell::TheShell->ifCommandStack.top();
      Shell::TheShell->ifCommandStack.top()->isWhile = true;

    } arg_list RBRACKET SEMI DO {
        //IfCommand* currentIfCommand = Shell::TheShell->ifCommandStack.top();
        //currentIfCommand->insertCondition(Shell::TheShell->_simpleCommand);
        Shell::TheShell->ifCommandStack.top()->insertCondition(Shell::TheShell->_simpleCommand);
	      Shell::TheShell->_simpleCommand = new SimpleCommand();

    } command_list DONE{
      Shell::TheShell->_level--; 
      //IfCommand* completedIfCommand = Shell::TheShell->ifCommandStack.top();
      //ListCommands* completedListCommands = Shell::TheShell->listCommandStack.top();
      //completedIfCommand->insertListCommands(completedListCommands);
      Shell::TheShell->ifCommandStack.top()->insertListCommands(Shell::TheShell->listCommandStack.top());
      Shell::TheShell->listCommandStack.pop();
      fprintf(stderr, "while parsed");
	    //Shell::TheShell->_listCommands = new ListCommands();
      //Shell::TheShell->listCommandStack.pop();

    }
    ;

for_command:
    FOR WORD IN arg_list SEMI DO command_list DONE
    ;

%%

void
yyerror(const char * s)
{
  fprintf(stderr,"%s", s);
}

#if 0
main()
{
  yyparse();
}
#endif
