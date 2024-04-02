#ifndef shell_hh
#define shell_hh

#include "ListCommands.hh"
#include "PipeCommand.hh"
#include "IfCommand.hh"
#include "Shell.hh"
#include <string>
#include <stack>

class Shell {

public:
  int _level; // Only outer level executes.
  bool _enablePrompt;
  ListCommands * _listCommands; 
  SimpleCommand *_simpleCommand;
  PipeCommand * _pipeCommand;
  IfCommand * _ifCommand;
  Command * _currentCommand;
  static Shell * TheShell;
  std::string question;
  std::string glob;
  int return_last_exit;
  int pid_background;
  std::stack<IfCommand*> ifCommandStack;
  std::stack<ListCommands*> listCommandStack;

  Shell();
  void execute();
  void print();
  void clear();
  void prompt();

};

#endif
