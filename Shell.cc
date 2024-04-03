
#include <unistd.h>
#include <cstdio>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include "Shell.hh"
#include "Command.hh"
#include "IfCommand.hh"
#include <sys/wait.h>
#include <stack>
int yyparse(void);
void yyrestart(FILE *file);
Shell * Shell::TheShell;

Shell::Shell() {
    this->_level = 0;
    this->_enablePrompt = true;
    this->_listCommands = new ListCommands(); 
    this->_simpleCommand = new SimpleCommand();
    this->_pipeCommand = new PipeCommand();
    this->_currentCommand = this->_pipeCommand;
    this->_ifCommand = new IfCommand();
    this->glob = "NULL";
    this->return_last_exit = 0;
    this->pid_background = 0;
    this->ifCommandStack = *(new std::stack<IfCommand*>());
    this->listCommandStack = *(new std::stack<ListCommands*>());
    this->listCommandStack.push(new ListCommands());
 
    this->num_args = 0;

    if ( !isatty(0)) {
	this->_enablePrompt = false;
    }
}

void Shell::prompt() {
    char * PROMPT  = getenv("PROMPT");
    if (_enablePrompt) {
    if (!PROMPT) {
	printf("myshell>");
	fflush(stdout);
  } else {
    printf("%s", PROMPT);
    fflush(stdout);
  }
    }
}

void Shell::print() {
    printf("\n--------------- Command Table ---------------\n");
    this->_listCommands->print();
}

void Shell::clear() {
    this->_listCommands->clear();
    this->_simpleCommand->clear();
    this->_pipeCommand->clear();
    this->_currentCommand->clear();
    this->_ifCommand->clear();
    this->_level = 0;
}

void Shell::execute() {
  //fprintf(stderr, "%d\n", Shell::TheShell->_level);
  if (this->_level == 0) {
    //this->print();
    this->_listCommands->execute();
    this->_listCommands->clear();
    this->prompt();
  }
}

void yyset_in (FILE *  in_str );

//Ctrl-c Helper
extern "C" void disp_ctrlc( int sig )
{
  fprintf(stderr, "\n");
  Shell::TheShell->clear();

  Shell::TheShell->prompt();
}
extern "C" void disp_zombie( int sig) {
  //wait3(0,0,NULL);
  while(waitpid(-1,NULL,WNOHANG) > 0);
}
int main(int argc, char **argv) {
  //Shell::TheShell->num_args = argc;
  //this->script_name = argv[0];
  char * input_file = NULL;
  if ( argc > 1 ) {
    input_file = argv[1];
    FILE * f = fopen(input_file, "r");
    if (f==NULL) {
	fprintf(stderr, "Cannot open file %s\n", input_file);
        perror("fopen");
        exit(1);
    }
    yyset_in(f);
  }  

  Shell::TheShell = new Shell();

  if (input_file != NULL) {
    // No prompt if running a script
    Shell::TheShell->_enablePrompt = false;
  }
  else {
    FILE *file = fopen(".shellrc", "r");
    if (file != NULL) {
        yyrestart(file);
        yyparse();
        yyrestart(stdin);
        fclose(file);
    } else {
        Shell::TheShell->prompt();
    }
  }
  Shell::TheShell->num_args = argc - 2;
  Shell::TheShell->script_name = argv[0];
    if (argc > 1) {
        Shell::TheShell->arg1 = argv[1];
    }
    if (argc > 2) {
        Shell::TheShell->arg2 = argv[2];
    }
    if (argc > 3) {
        Shell::TheShell->arg3 = argv[3];
    }
    if (argc > 4) {
        Shell::TheShell->arg4 = argv[4];
    }
  //Ctrl-c Implementation
  struct sigaction sa;
  sa.sa_handler = disp_ctrlc;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;

  if(sigaction(SIGINT, &sa, NULL)){
    perror("sigaction");
    exit(-1);
  }

  //Zombie Process Implementation
  struct sigaction sa_zombie;
  sa_zombie.sa_handler = disp_zombie;
  sigemptyset(&sa_zombie.sa_mask);
  sa_zombie.sa_flags = SA_RESTART;

  int error = sigaction(SIGCHLD, &sa_zombie, NULL); 

  if (error) {
    perror ("sigaction");
    exit(-1);

  }


  yyparse();
}


