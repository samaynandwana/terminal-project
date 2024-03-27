
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "Command.hh"
#include "SimpleCommand.hh"
#include "IfCommand.hh"

IfCommand::IfCommand() {
    _condition = NULL;
    _listCommands =  NULL;
}


// Run condition with command "test" and return the exit value.


/*int
IfCommand::runTest(SimpleCommand * condition) {
    /*condition->print();
    return 1;*/
    /*std::vector<char*> args;
    for (std::string* arg : condition->_arguments) {
      args.push_back(const_cast<char*>(arg->c_str()));
    }
    args.push_back(nullptr);
    int ret = fork();
    if (ret < 0) {
      perror("fork");
      return 1;
    } else if (ret == 0) {
      execvp("test", args.data());
      perror("execvp");
      exit(1);
    } else {
        int status;
        waitpid(ret, &status, 0);
        if (WIFEXITED(status)) {
            fprintf(stderr, "W STATUS:%d\n", WEXITSTATUS(status));
            return WEXITSTATUS(status);
        } else {
            fprintf(stderr, "reached return 1\n");
            return 1;
        }
    }
}*/
/*int IfCommand::runTest(SimpleCommand * condition) {
    std::string line = "test";

    for (std::string* arg : condition->_arguments) {
        line += " " + *arg;
    }

    char *args[] = {"/bin/sh", "-c", const_cast<char*>(line.c_str()), nullptr};

    int ret = fork();
    if (ret < 0) {
        perror("fork");
        return 1;
    } else if (ret == 0) {
        execvp(args[0], args);
        perror("execvp");
        exit(1);
    } else {
        int status;
        waitpid(ret, &status, 0);
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            return 1;
        }
    }
}*/
int IfCommand::runTest(SimpleCommand * condition) {
    std::string commandLine = "test";
    for (std::string* arg : condition->_arguments) {
        commandLine += " " + *arg;
    }
    commandLine += "\n";

    int pin[2], pout[2];
    pipe(pin);
    pipe(pout);

    int ret = fork();
    if (ret == 0) {
        dup2(pin[0], 0);
        dup2(pout[1], 1);
        close(pin[1]);
        close(pout[0]);
        close(pin[0]);
        close(pout[1]);

        char *argv[] = {"/proc/self/exe", const_cast<char*>(commandLine.c_str()), nullptr};
        execvp(argv[0], (char* const*)argv);
        perror("execvp");
        _exit(1);
    } else {
        write(pin[1], commandLine.c_str(), commandLine.size());
        close(pin[0]);
        close(pin[1]);

        close(pout[1]);
        int status;
        waitpid(ret, &status, 0);

        if (WIFEXITED(status)) {
            close(pout[0]);
            return WEXITSTATUS(status);
        } else {
            close(pout[0]);
            return 1;
        }
    }
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
    if (runTest(this->_condition) == 0) {
	_listCommands->execute();
    }
}

