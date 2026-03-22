#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

#define MAX_INT 2147483647

int main(){

    char input[1024];
    char delim = ';';
    char *token;
    char argv[1024] = {};

    printf("hello. welcome to the shell\n");

    while (fgets(input, sizeof(input), stdin) != NULL)
    {
        printf("i read this: %s\n",input);

        token = strtok(input, &delim);

        int i = 0;
        argv[i] = *token;
        i++;
        while (token != NULL)
        {
            printf("current token: %s\n",token);

            token = strtok(NULL, &delim); //need this NULL call to keep going, according to strtok stackoverflow

            argv[i] = *token;
            i++;
        }
        argv[i] = NULL;

        //fork, execvp, wait
        int c_pid = 0;
        c_pid = fork();
        if (c_pid == 0) //child
        {
            printf("this is the child\n");

            int iserr = 0;
            iserr = execvp(&argv[0],&argv);

            if (iserr == -1) 
            {
                perror("problem with execvp, argv[]\n");
            }
        }
        else if (c_pid == -1) //error
        {
            perror("error with child process creation!\n");
        }
        else //parent
        {
            printf("in the parent, PID = %d\n",c_pid);
            waitpid(c_pid); //error
        }
    }


    return 0;
}