#ifndef pipecommand_hh
#define pipecommand_hh

#include "Command.hh"
#include "SimpleCommand.hh"

// Command Data Structure

class PipeCommand : public Command {
public:
  std::vector<SimpleCommand *> _simpleCommands;
  bool append_out;
  std::string * _outFile;
  std::string * _inFile;
  std::string * _errFile;
  bool append_err;
  bool _background;

  PipeCommand();
  void insertSimpleCommand( SimpleCommand * simpleCommand );

  void clear();
  void print();
  void execute();

  // Expands environment vars and wildcards of a SimpleCommand and
  // returns the arguments to pass to execvp.
  char ** expandEnvVarsAndWildcards(SimpleCommand * simpleCommandNumber);

};

#endif
