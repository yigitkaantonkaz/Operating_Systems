//Yigit Kaan Tonkaz
//29154
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <assert.h>

int main(int argc, const char * argv[]) {
    
    printf("I'm SHELL process, with PID: %d - Main command is: man ls | grep -A 1 -m 1 -- '-h' \n", getpid());
    
    int pipefd[2];
    pipe(pipefd);
    
    if(pipe(pipefd) == -1){
        perror("Pipe failed");
        exit(1);
    }
    
    int pid1 = fork();
    
    //fork failed
    if(pid1 < 0){
        fprintf(stderr, "First fork failed\n");
        exit(1);
    }
    else if(pid1 == 0){
        //child 1 which is man
        printf("I’m MAN process, with PID: %d - My command is: man ls \n", getpid());
        
        dup2(pipefd[1], STDOUT_FILENO); // write end of the pipe
        close(pipefd[0]);// Close read end of the pipe
        
        execlp("man", "man", "ls", NULL);
        
        perror("execlp (man) failed");
        exit(1);
    }
    else{//parent
        //child 2 which is grep
        int pid2 = fork();
        
        if(pid2 < 0){
            fprintf(stderr, "Second fork failed\n");
            exit(1);
        }
        else if(pid2 == 0){
            printf("I’m GREP process, with PID: %d - My command is: grep -A 1 -m 1 -- '-h' \n", getpid());
            
            int output_file = open("output.txt", O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
            dup2(output_file, STDOUT_FILENO);
            close(output_file);
            
            dup2(pipefd[0], STDIN_FILENO);// read end of the pipe
            close(pipefd[0]);
            close(pipefd[1]);// Close write end of the pipe
            
            execlp("grep", "grep", "-A", "1", "-m", "1", "--", "-h", NULL);
            
            perror("execlp (grep) failed");
            exit(1);
        }
        else{
            close(pipefd[0]);
            close(pipefd[1]);
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);
            //parent
            printf("I’m SHELL process, with PID: %d - execution is completed, you can find the results in output.txt \n", getpid());
        }
    }
    return 0;
}
