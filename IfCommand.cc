
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include "Command.hh"
#include "SimpleCommand.hh"
#include "PipeCommand.hh"
#include "IfCommand.hh"
#include "Shell.hh"

extern char ** array;
int maxEntries;
int nEntries;
IfCommand::IfCommand() {
    _condition = NULL;
    _listCommands =  NULL;
    isWhile = false;
    isFor = false;
}


// Run condition with command "test" and return the exit value.


int IfCommand::runTest(SimpleCommand * condition) {
    SimpleCommand* copy = new SimpleCommand();
    for (const std::string* arg : condition->_arguments) {
        copy->insertArgument(new std::string(*arg));
    }
    if (copy->_arguments.empty() || *copy->_arguments[0] != "test") {
        copy->_arguments.insert(copy->_arguments.begin(), new std::string("test"));
    }
    PipeCommand* pipe = new PipeCommand();
    pipe->insertSimpleCommand(copy);
    pipe->execute();

    delete copy;
    return Shell::TheShell->return_last_exit;

}

int IfCommand::runForTest(SimpleCommand * condition) {
    SimpleCommand* copy = new SimpleCommand();
    for (const std::string* arg : condition->_arguments) {
        copy->insertArgument(new std::string(*arg));
    }
    /*if (copy->_arguments.empty() || *copy->_arguments[0] != "test") {
        copy->_arguments.insert(copy->_arguments.begin(), new std::string("test"));
    }*/
    PipeCommand* pipe = new PipeCommand();
    pipe->insertSimpleCommand(copy);
    pipe->execute();

    delete copy;
    return Shell::TheShell->return_last_exit;

}



void 
IfCommand::insertCondition( SimpleCommand * condition ) {
    _condition = condition;
}

void 
IfCommand::insertListCommands( ListCommands * listCommands) {
    _listCommands = listCommands;
}

void 
IfCommand::clear() {
}

void 
IfCommand::print() {
    printf("IF [ \n"); 
    this->_condition->print();
    printf("   ]; then\n");
    this->_listCommands->print();
}
  
void 
IfCommand::execute() {
    // Run command if test is 0
    int count = 1;
    if(isWhile) {
      while(runTest(this->_condition) == 0) {
        _listCommands->execute();
        count++;
      }
    }
    if (isFor) {
      SimpleCommand* copy = new SimpleCommand();
      for (const std::string* arg : _condition->_arguments) {
        copy->insertArgument(new std::string(*arg));
      }
      PipeCommand* pipe = new PipeCommand();
      pipe->insertSimpleCommand(copy);
      //pipe->expandWildcards();
      //pipe->execute();
      bool wildcard = false;
      for (unsigned long j = 0; j < copy->_arguments.size(); j++) {
        std::string& arg = *copy->_arguments[j];
        if (arg.find('*') != std::string::npos || arg.find('?') != std::string::npos) {
          wildcard = true;
          break;
        }
      }
      if (wildcard) {
          for (unsigned long j = 0; j < copy->_arguments.size(); j++) {
            std::string& arg = *copy->_arguments[j];
            if ((arg.find('*') == std::string::npos && arg.find('?') == std::string::npos)) {
              continue;
            } else {
              pipe->expandWildcard(NULL, (char *) arg.c_str(), true);
              copy->_arguments.erase(copy->_arguments.begin() + j);
              sortArray(array, nEntries);
              if (nEntries == 0) {
                copy->insertArgument(new std::string(arg.c_str()));
              }
              for (int b = 0; b < nEntries; b++) {
                 std::string* app = new std::string(array[b]);
                 if (!app->empty() && (*app)[0] == '/') {
                     *app = app->substr(1);
                 }
                  else if (app->size() >= 2 && (*app)[0] == '.' && (*app)[1] == '/') {
                    *app = app->substr(2);
                  }

                copy->insertArgument(app);
              }
            }
          }
      }


       std::vector<std::string> argVals;
       for (const std::string *arg : _condition->_arguments) {
         argVals.push_back(*arg);
       }
       for (auto arg : argVals) {
            std::string str = arg;
            setenv(this->loop_var.c_str(), str.c_str(), 1);
            //runForTest(_condition);
            _listCommands->execute();
        }
        /*std::string loop;
        std::vector<std::string> items;
        for (const std::string& item : items) {
          setenv(loop.c_str(), item.c_str(), 1);
          PipeCommand *pipe = new PipeCommand();
          pipe->insertSimpleCommand(_condition);
          pipe->execute();

          _listCommands->execute();
        }*/
    }
    else {
    if (runTest(this->_condition) == 0) {
        _listCommands->execute();
    }
    }
}
void PipeCommand::sortArray(char **array, int nEntries) {
    int i, j;
    for (i = 0; i < nEntries - 1; i++) {
        for (j = 0; j < nEntries - i - 1; j++) {
            if (strcmp(array[j], array[j + 1]) > 0) {
                char *temp = array[j];
                array[j] = array[j + 1];
                array[j + 1] = temp;
            }
        }
    }
}

