
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
    //pipe->execute();

    //delete copy;

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

