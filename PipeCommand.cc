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
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <sstream>
#include "PipeCommand.hh"
#include "Shell.hh"
#include "y.tab.hh"
#include <sys/types.h>
#include <pwd.h>
#include <regex.h>
#include <dirent.h>
#include <algorithm>
#include <cassert>
#define MAXFILENAME 1024
PipeCommand::PipeCommand() {
    // Initialize a new vector of Simple PipeCommands
    _simpleCommands = std::vector<SimpleCommand *>();

    _outFile = NULL;
    _inFile = NULL;
    _errFile = NULL;
    _background = false;
    //question = NULL;
}

void PipeCommand::insertSimpleCommand( SimpleCommand * simplePipeCommand ) {
    // add the simple command to the vector
    _simpleCommands.push_back(simplePipeCommand);
}
//int maxEntries = 20;
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
int maxEntries = 20;
int nEntries = 0;
char ** array;
void PipeCommand::execute() {
    // Don't do anything if there are no simple commands
    if ( _simpleCommands.size() == 0 ) {
        Shell::TheShell->prompt();
        return;
    }
    if (strcmp(_simpleCommands[0]->_arguments[0]->c_str(),"exit") == 0) {
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
        //fderr = dup(tmperr);
      }
      //close file descriptors
      dup2(fdout, 1);
      close(fdout);
      dup2(fderr, 2);
      close(fderr);
      //implementation of CD
      if (!strcmp(_simpleCommands[0]->_arguments[0]->c_str(), "cd")) {
        if (_simpleCommands[0]->_arguments[1]) {
          int is_error = 0; //variable for storing the return of chdir
          //case for trying to cd into the expanded env variable of ${HOME}
          if(strcmp(_simpleCommands[0]->_arguments[1]->c_str(),"${HOME}") == 0) {
            char *dir = getenv("HOME");
            is_error = chdir(dir);
          //cd to the specified directory
          } else {
            is_error = chdir(_simpleCommands[0]->_arguments[1]->c_str());
          }
          //error directory not found
          if (is_error != 0 ) {
            fprintf(stderr, "cd: can't cd to %s", _simpleCommands[0]->_arguments[1]->c_str());
          }
        //cd home, because no directory was specified
        } else {
          char *dir = getenv("HOME");
          chdir(dir);
        }
        continue;
      }
      //implementation for setenv, set an environment variable with C's setenv function
      extern char ** environ;
      //child process create with fork
            if (!strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "setenv")) {
        setenv(_simpleCommands[i]->_arguments[1]->c_str(), _simpleCommands[i]->_arguments[2]->c_str(), 1);
        continue;
        //exit(0);
      }
      //implementation for unsetenv, unset a passed in environment variable
      if (!strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "unsetenv")) {
        unsetenv(_simpleCommands[i]->_arguments[1]->c_str());
        continue;
      }
      //Tilde Expansion
      for (unsigned long j = 0; j < _simpleCommands[i]->_arguments.size(); j++) {
          std::string& arg = *_simpleCommands[i]->_arguments[j];
          //fprintf(stderr, "Arg:%s\n", arg.c_str());
          //check if there is a tilde
          if (arg[0] == '~') {
            //case where nothing is specified, expand to current user's home
            if (arg.length() == 1 || arg[1] == '/') {
              const char* homeDir = getenv("HOME");
              if (homeDir != nullptr) {
                arg = std::string(homeDir) + arg.substr(1);
              }
            //case where you expand the tilde to the specified user's home
            } else {
              size_t slashPos = arg.find('/');
              std::string username;
              if (slashPos != std::string::npos) {
                username = arg.substr(1, slashPos - 1);
              } else {
                username = arg.substr(1);
              }
              struct passwd* pw = getpwnam(username.c_str());
              if (pw != nullptr) {
                std::stringstream ss;
                ss << pw->pw_dir;
                if (slashPos != std::string::npos) {
                    ss << arg.substr(slashPos);
                    arg = ss.str();
                } else {
                    arg = std::string(pw->pw_dir);
                }
               }
            }
         }
         }

      //Subshell Implementation
      for (unsigned long k = 0; k < _simpleCommands[i]->_arguments.size(); k++) {
        bool modify = false; //will be updated if we need to parse subshell
        std::string& str = *_simpleCommands[i]->_arguments[k];
        //parsing for the $() case
        if (str.front() == '$' && str[1] == '(' && str.back() == ')') {
            str = str.substr(2, str.length() - 3);
            modify = true;
        //parsing for the '' case
        } else if (str.front() == '\'' && str.back() == '\'') {
            str = str.substr(2, str.length() - 3);
            modify = true;
        } else {
            continue;
        }
        if (modify) {
          //set up pipes for subshell
          int pin[2];
          int pout[2];
          pipe(pin);
          pipe(pout);
          int sub_ret = fork(); //subshell fork call
          if (sub_ret == 0) {
            dup2(pin[0], 0);
            dup2(pout[1], 1);
            close(pin[1]);
            close(pout[0]);
            close(pin[0]);
            close(pout[1]);
            const char *argv[] = {"/proc/self/exe", NULL};
            execvp(argv[0], (char* const*)argv); //call execvp child process
            _exit(1); //exit the new subshell process
          } else {
            //read in the contents of the subshell output
            write(pin[1], str.c_str(), str.size());
            write(pin[1], "\n", 1);
            close(pin[0]);
            close(pin[1]);
            close(pout[1]);
            char c;
            std::vector<char> buffer;
            //push contents into a buffer array
            while (read(pout[0], &c, 1) > 0) {
              if (c == '\n') {
                buffer.push_back(' ');
              } else {
                buffer.push_back(c);
              }
            }
            //split the buffer array by space
            std::string buffstr(buffer.begin(), buffer.end());
            std::vector<std::string> words;
            std::stringstream ss(buffstr);
            std::string word;
            while (ss >> word) {
              words.push_back(word);
            }
            //pass in the arguments one by one from buffer
            _simpleCommands[i]->_arguments[k] = new std::string(words[0]);
            for (unsigned long a = 1; a < words.size(); a++) {
                  _simpleCommands[i]->insertArgument(new std::string(words[a]));

            }
          }
         }
      }
      //Wildcarding Implementation
      bool wildcard = false;
      for (unsigned long j = 0; j < _simpleCommands[i]->_arguments.size(); j++) {
        std::string& arg = *_simpleCommands[i]->_arguments[j];
        if (arg.find('*') != std::string::npos || arg.find('?') != std::string::npos) {
          wildcard = true;
          break;
        }
      }
      if (wildcard) {
          for (unsigned long j = 0; j < _simpleCommands[i]->_arguments.size(); j++) {
            std::string& arg = *_simpleCommands[i]->_arguments[j];
            if ((arg.find('*') == std::string::npos && arg.find('?') == std::string::npos)) {
              continue;
            } else {
              expandWildcard(NULL, (char *) arg.c_str());
              _simpleCommands[i]->_arguments.erase(_simpleCommands[i]->_arguments.begin() + j);
              sortArray(array, nEntries);
              for (int b = 0; b < nEntries; b++) {
                _simpleCommands[i]->insertArgument(new std::string(array[b]));
              }
            }
          }
      }
      const char ** args = (const char **) malloc ((_simpleCommands[i]->_arguments.size() + 1)*sizeof(char*));
      for (unsigned long j = 0; j < _simpleCommands[i]->_arguments.size(); j++) {
        args[j] = _simpleCommands[i]->_arguments[j]->c_str();
      }
      args[_simpleCommands[i]->_arguments.size()] = NULL;

      //Environment Variable Expansion
      for (unsigned long j = 0; j < _simpleCommands[i]->_arguments.size(); j++) {
          std::string& arg = *_simpleCommands[i]->_arguments[j];
          //parsing to see if there is an env variable
          std::size_t start_pos = arg.find("${");
          while (start_pos != std::string::npos) {
            std::size_t end_pos = arg.find("}", start_pos);
            if (end_pos != std::string::npos) {
              //get the contents of the env variable
              std::string envv = arg.substr(start_pos + 2, end_pos - start_pos - 2);
              char *env_val = getenv(envv.c_str());
              //Special cases for expansion
              if (!strcmp(envv.c_str(), "SHELL")) {
                char *path = realpath("../lab3-src/shell", NULL);
                args[j] = path;
              } else if (!strcmp(envv.c_str(), "$")) {
                args[j] = (std::to_string(getpid())).c_str();
              } else if (!strcmp(envv.c_str(), "_")) {
                args[j] = Shell::TheShell->glob.c_str();
              } else if (!strcmp(envv.c_str(), "!")) {
                args[j] = (std::to_string(Shell::TheShell->pid_background)).c_str();
              } else if (!strcmp(envv.c_str(), "?")) {
                args[j] = (std::to_string(Shell::TheShell->return_last_exit)).c_str();
              } else {
                //base case for expansion
                if (env_val != NULL) {
                  args[j] = env_val;
                }
              }
              //update the starting position
              std::string copy = envv.c_str();
              start_pos = arg.find("${", start_pos + copy.length());
            }
          }
      }


      ret = fork();
      if (ret == 0) {
        std::vector<char *> env_arg;
        //print env implementation
        if (!strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "printenv")) {
          char **p = environ;
          while (*p != NULL) {
            printf("%s\n", *p);
            p++;
          }
          exit(0);
        }
        //call execvp
        execvp(args[0], (char* const*)args);
        perror("execvp");
        exit(1);
      }
      Shell::TheShell->glob = std::string(_simpleCommands[i]->_arguments.back()->c_str());
    }
    //close temps
    dup2(tmpin, 0);
    dup2(tmpout, 1);
    dup2(tmperr, 2);
    close(tmpin);
    close(tmpout);
    close(tmperr);

    if (!_background) {
      int i;
      waitpid(ret, &i, 0);
      Shell::TheShell->return_last_exit = WEXITSTATUS(i);
    } else {
      Shell::TheShell->pid_background = ret;
    }
    // Clear to prepare for next command
    clear();

    // Print new prompt
    //Shell::TheShell->prompt();
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

/* Function for expanding a wildcard, where prefix is already expanded 
and suffix may still contain wildcards
*/
void PipeCommand::expandWildcard(char *prefix, char *suffix) {
          //recursion base case when the whole thing is expanded
          if (suffix[0] == 0) {
            array[nEntries] = strdup(prefix);
            nEntries++;
            return;
          }
          //modify suffix based on subpaths
          char * s = strchr(suffix, '/');
          char component[MAXFILENAME];
          if (s != NULL) { //copy up to '/'
            strncpy(component, suffix, s - suffix);
            component[s - suffix] = '\0';
            suffix = s + 1;
          } else { //whole thing
            strcpy(component, suffix);
            suffix = suffix + strlen(suffix);
          }
          char * reg = (char*)malloc(2*strlen(component)+10);
          const char * a = component;
          char * r = reg;
          *r = '^'; r++; // match beginning of line
          while (*a) {
            if (*a == '*') { *r='.'; r++; *r='*'; r++; }
            else if (*a == '?') { *r='.'; r++;}
            else if (*a == '.') { *r='\\'; r++; *r='.'; r++;}
            else { *r=*a; r++;}
            a++;
          }
          *r='$'; r++; *r=0;
          regex_t re;
          int expbuf = regcomp(&re, reg, REG_EXTENDED|REG_NOSUB);
          if (expbuf != 0) {
            perror("compile");
            return;
          }
          DIR *dir = opendir(".");
          if (dir == NULL) {
            perror("opendir");
            return;
          }
          struct dirent *ent;
          maxEntries = 20;
          nEntries = 0;
          regmatch_t match;
          array = (char **) malloc(maxEntries*sizeof(char *));
          while ((ent = readdir(dir)) != NULL) {
            if (regexec(&re, ent->d_name, 1, &match, 0) == 0) {
              if (nEntries == maxEntries) {
                maxEntries *= 2;
                array = (char **)realloc(array, maxEntries*sizeof(char *));
                assert(array != NULL);
              }
              if (ent->d_name[0] == '.') {
                if (suffix[0] == '.') {
                  array[nEntries] = strdup(ent->d_name);
                  nEntries++;

                }
              } else {
                 array[nEntries] = strdup(ent->d_name);
                 nEntries++;
                }
              }
            }
          closedir(dir);

}
// Expands environment vars and wildcards of a SimpleCommand and
// returns the arguments to pass to execvp.
char ** 
PipeCommand::expandEnvVarsAndWildcards(SimpleCommand * simpleCommandNumber)
{
    simpleCommandNumber->print();
    return NULL;
}


