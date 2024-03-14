//yigit kaan 29154
#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <sstream>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/wait.h>
#include <fstream>
#include <vector>

using namespace std;

void logCommandInfo(const vector<string>& tokens, bool hasRedirection, bool isBackground) {
    ofstream logf("parse.txt", ios::app);

    if (!logf.is_open()) {
        return;
    }
    logf << "----------" << endl;
    logf << "Command: " << tokens[0] << endl;
    if (tokens.size() > 1) {
        logf << "Input: " << tokens[1] << endl;
    }
    if (tokens.size() > 2) {
        logf << "Option: " << tokens[2] << endl;
    }
    logf << "Redirection: " << (hasRedirection ? ">" : "-") << endl;
    logf << "Background: " << (isBackground ? "y" : "n") << endl;
    logf << "----------" << endl;

    logf.flush();
    logf.close();
}

void* listener(void* arg) {
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    
    int fd = *((int*)arg);
    pthread_mutex_lock(&lock);
    cout << "---- " << pthread_self() << endl;
    char buffer[1024];
    ssize_t bytes_read;
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        write(STDOUT_FILENO, buffer, bytes_read);
    }
    cout << "---- " << pthread_self() << endl;
    pthread_mutex_unlock(&lock);
    return nullptr;
}

void executeCommand(const vector<string>& tokens) {
    int redirectInput = -1, redirectOutput = -1;
    vector<int> id_process;
    vector<pthread_t> id_thread;
    char redirect = '-';
    bool runInBackground = false;
    bool hasRedirection = false;
    bool isBackgroundJob = false;
    
    for (const string& token : tokens) {
        if (token == ">" || token == "<") {
            hasRedirection = true;
        } else if (token == "&") {
            isBackgroundJob = true;
        }
    }

    logCommandInfo(tokens, hasRedirection, isBackgroundJob);
    
    if (hasRedirection){
        int pid_rd = fork();

        if (pid_rd < 0) {
            cout << "Fork failed" << endl;
        } else if (pid_rd == 0) {
            redirectOutput = open(tokens[1].c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            dup2(redirectOutput, STDOUT_FILENO);
            close(redirectOutput);
            const char** args = new const char*[tokens.size() + 1];
            for (size_t i = 0; i < tokens.size(); ++i) {
                args[i] = tokens[i].c_str();
            }
            args[tokens.size()] = nullptr;
            execvp(args[0], const_cast<char**>(args));
            delete[] args;
        } else {
            if (!isBackgroundJob) {
                waitpid(pid_rd, NULL, NULL);
            } else {
                id_process.push_back(pid_rd);
            }
        }
    }
    else {
        int fd[2];
        pipe(fd);
        pthread_t thread;
        
        int pid_parent = fork();
        
        if (pid_parent < 0) {
            cout << "Failed to fork" << endl;
        } else if (pid_parent == 0) {
            if (tokens[1] == "<") {
                redirectInput = open(tokens[2].c_str(), O_RDONLY);
                dup2(redirectInput, STDIN_FILENO);
                close(redirectInput);
            }
            close(fd[0]);
            dup2(fd[1], STDOUT_FILENO);
            close(fd[1]);
            const char** args = new const char*[tokens.size() + 1];
            for (size_t i = 0; i < tokens.size(); ++i) {
                args[i] = tokens[i].c_str();
            }
            args[tokens.size()] = nullptr;
            execvp(args[0], const_cast<char**>(args));
            delete[] args;
        } else {
            close(fd[1]);
            pthread_create(&thread, nullptr, listener, &fd[0]);
            if (isBackgroundJob) {
                id_process.push_back(pid_parent);
                id_thread.push_back(thread);
            } else {
                waitpid(pid_parent, NULL, NULL);
                pthread_join(thread, nullptr);
            }
        }
    }
    
    for (int i=0; i < id_process.size(); i++){
        waitpid(id_process[i], NULL, NULL);
    }
    
    for(int i=0; i <  id_thread.size(); i++){
        pthread_join(id_thread[i], NULL);
    }
    
    id_process.clear();
    id_thread.clear();
}

vector<string> parseCommand(const string& command) {
    vector<string> tokens;
    istringstream iss(command);
    string token;

    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

int main(int argc, const char * argv[]) {
    ifstream f;
    f.open("commands.txt");
    ofstream parse;
    parse.open("parse.txt");
    
    if (!f.is_open()) {
        return EXIT_FAILURE;
    }
    string line;
    while (getline(f, line)) {
        vector<string> tokens = parseCommand(line);
        if (!tokens.empty()) {
            executeCommand(tokens);
        }
    }
    parse.close();
    f.close();
    
    return EXIT_SUCCESS;
}
