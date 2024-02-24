/*
 * CS252: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 * DO NOT PUT THIS PROJECT IN A PUBLIC REPOSITORY LIKE GIT. IF YOU WANT 
 * TO MAKE IT PUBLICALLY AVAILABLE YOU NEED TO REMOVE ANY SKELETON CODE 
 * AND REWRITE YOUR PROJECT SO IT IMPLEMENTS FUNCTIONALITY DIFFERENT THAN
 * WHAT IS SPECIFIED IN THE HANDOUT. WE OFTEN REUSE PART OF THE PROJECTS FROM  
 * SEMESTER TO SEMESTER AND PUTTING YOUR CODE IN A PUBLIC REPOSITORY
 * MAY FACILITATE ACADEMIC DISHONESTY.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cstring>

#include "PipeCommand.hh"
#include "Shell.hh"


PipeCommand::PipeCommand() {
    // Initialize a new vector of Simple PipeCommands
    _simpleCommands = std::vector<SimpleCommand *>();

    _outFile = NULL;
    _inFile = NULL;
    _errFile = NULL;
    _background = false;
}

void PipeCommand::insertSimpleCommand( SimpleCommand * simplePipeCommand ) {
    // add the simple command to the vector
    _simpleCommands.push_back(simplePipeCommand);
}

void PipeCommand::clear() {
    // deallocate all the simple commands in the command vector
    for (auto simplePipeCommand : _simpleCommands) {
        delete simplePipeCommand;
    }

    // remove all references to the simple commands we've deallocated
    // (basically just sets the size to 0)
    _simpleCommands.clear();
    if (_outFile == _errFile) {
      delete _outFile;
      _outFile = NULL;
      _errFile = NULL;
    }
    if ( _outFile ) {
        delete _outFile;
    }
    _outFile = NULL;

    if ( _inFile ) {
        delete _inFile;
    }
    _inFile = NULL;

    if ( _errFile ) {
        delete _errFile;
    }
    _errFile = NULL;

    _background = false;
}

void PipeCommand::print() {
    printf("\n\n");
    //printf("              COMMAND TABLE                \n");
    printf("\n");
    printf("  #   Simple PipeCommands\n");
    printf("  --- ----------------------------------------------------------\n");

    int i = 0;
    // iterate over the simple commands and print them nicely
    for ( auto & simplePipeCommand : _simpleCommands ) {
        printf("  %-3d ", i++ );
        simplePipeCommand->print();
    }

    printf( "\n\n" );
    printf( "  Output       Input        Error        Background\n" );
    printf( "  ------------ ------------ ------------ ------------\n" );
    printf( "  %-12s %-12s %-12s %-12s\n",
            _outFile?_outFile->c_str():"default",
            _inFile?_inFile->c_str():"default",
            _errFile?_errFile->c_str():"default",
            _background?"YES":"NO");
    printf( "\n\n" );
}

void PipeCommand::execute() {
    // Don't do anything if there are no simple commands
    if ( _simpleCommands.size() == 0 ) {
        Shell::TheShell->prompt();
        return;
    }
    if (strcmp(_simpleCommands[0]->arguments[0],"exit") == 0) {
      fprintf(stderr, "Good bye!!\n");
      exit(1);
    }
    // Print contents of PipeCommand data structure
    //print();

    // Add execution here
    // For every simple command fork a new process

    //temp file descriptors
    int tmpin = dup(0);
    int tmpout = dup(1);
    int tmperr = dup(2);
    int fdin;
    //open the infile
    if (_inFile) {
      //open file
      fdin = open(_inFile->c_str(), O_RDONLY, 0777);
    } else {
      fdin = dup(tmpin);
    }
    //fdin will contain -1 if file did not exist, so return error as follows
    if (fdin == -1) {
      fprintf(stderr,"/bin/sh: 1: cannot open %s: No such file\n", _inFile->c_str()); 
      clear();
    }
    int ret;
    int fdout;
    int fderr;
    //loop through the vector of simple commands
    for (unsigned long i = 0; i < _simpleCommands.size(); i++) {
      dup2(fdin, 0);
      close(fdin);
      //last argument
      if (i == _simpleCommands.size() - 1) {
        if (_outFile) {
          //open outfile, but also have to check append and error conditions
          if (append_out) {
            fdout = open(_outFile->c_str(), O_APPEND |  O_WRONLY, 0777);
          } else {
            fdout = open(_outFile->c_str(), O_CREAT | O_TRUNC | O_WRONLY, 0777);
          }
        } else {
          fdout = dup(tmpout);
        }
        //open the error file and also check the append and error conditions
        if (_errFile) {
          if (append_err) {
            fderr = open(_errFile->c_str(), O_APPEND | O_WRONLY, 0777);
          } else {
            fderr = open(_errFile->c_str(), O_CREAT | O_TRUNC | O_WRONLY, 0777);
          }

        } else {
          fderr = dup(tmperr);
        }
      //not last argument, proceed with piping
      } else {
        int fdpipe[2];
        pipe(fdpipe);
        fdout = fdpipe[1];
        fdin = fdpipe[0];
        fderr = dup(tmperr);
      }
      //close file descriptors
      dup2(fdout, 1);
      close(fdout);
      dup2(fderr, 2);
      close(fderr);
      //child process create with fork
      const char ** args = (const char **) malloc ((_simpleCommands[i]->_arguments.size() + 1)*sizeof(char*));
      for (unsigned long j = 0; j < _simpleCommands[i]->_arguments.size(); j++) {
        args[j] = _simpleCommands[i]->_arguments[j]->c_str();
      }
      args[_simpleCommands[i]->_arguments.size()] = NULL;
      ret = fork();
      if (ret == 0) {
        //call execvp
        execvp(args[0], (char* const*)args);
        perror("execvp");
        exit(1);
      }
    }
    //close temps
    dup2(tmpin, 0);
    dup2(tmpout, 1);
    dup2(tmperr, 2);
    close(tmpin);
    close(tmpout);
    close(tmperr);

    if (!_background) {
      waitpid(ret, NULL, 0);
    }
    // Clear to prepare for next command
    clear();

    // Print new prompt
    //Shell::TheShell->prompt();
}

// Expands environment vars and wildcards of a SimpleCommand and
// returns the arguments to pass to execvp.
char ** 
PipeCommand::expandEnvVarsAndWildcards(SimpleCommand * simpleCommandNumber)
{
    simpleCommandNumber->print();
    return NULL;
}


