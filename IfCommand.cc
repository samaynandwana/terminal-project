
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

IfCommand::IfCommand() {
    _condition = NULL;
    _listCommands =  NULL;
}


// Run condition with command "test" and return the exit value.

/*SimpleCommand* deepCopySimpleCommand(const SimpleCommand* original) {
    SimpleCommand* copy = new SimpleCommand();
    for (const std::string* arg : original->_arguments) {
        copy->insertArgument(new std::string(*arg));
    }
    return copy;
}*/

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

    /*std::string *commandLine = new std::string("test");
    condition->_arguments.insert(condition->_arguments.begin(), commandLine);
    condition->_arguments[0] = commandLine;*/
    /*for (std::string* arg : condition->_arguments) {
        commandLine += " " + *arg;
    }
    commandLine += "\n";*/
    /*    int ret = fork();
    if (ret == 0) {
        const char ** args = (const char **) malloc ((condition->_arguments.size() + 2)*sizeof(char*));
        for (unsigned long j = 0; j < condition->_arguments.size(); j++) {
          args[j + 1] = condition->_arguments[j]->c_str();
        }
        std::string first = "test";
        args[0] = first.c_str();
        args[condition->_arguments.size() + 1] = NULL;
        execvp(args[0], (char* const*)args);
        _exit(1);
    } else {
        int status;
        waitpid(ret, &status, 0);

        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            return 1;
        }
    }*/

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
    if (isWhile) {
      Shell::TheShell->_level++;
      //std::cerr << "Running while:\n";
      while(runTest(this->_condition) == 0) {
        _listCommands->execute();
        //fprintf(stderr, "while executed %d times\n", count);
        //count;
      }
      Shell::TheShell->_level--;
    } 
    else {
    if (runTest(this->_condition) == 0) {
	_listCommands->execute();
  }
    }
}

