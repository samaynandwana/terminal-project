#ifndef ifcommand_hh
#define ifcommand_hh

#include "Command.hh"
#include "SimpleCommand.hh"
#include "ListCommands.hh"

// Command Data Structure

class IfCommand : public Command {
public:
  SimpleCommand * _condition;
  ListCommands * _listCommands; 
  bool isWhile;
  bool isFor;
  IfCommand();
  void insertCondition( SimpleCommand * condition );
  void insertListCommands( ListCommands * listCommands);
  static int runTest(SimpleCommand * condition);
  //SimpleCommand * deepCopySimpleCommand(const * SimpleCommand original);

  void clear();
  void print();
  void execute();

};

#endif
