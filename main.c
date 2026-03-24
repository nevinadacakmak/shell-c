#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

#define MAX_INT 2147483647

int main(){

    char input[1024] = {};
    char *delim = " ";
    char *token;
    char *argv[1024] = {};

    printf("hello. welcome to the shell\n");

    while (fgets(input, sizeof(input), stdin) != NULL)
    {
        input[strcspn(input, "\n")] = '\0';
        //issue1: what if someone wants to put multiple lines?
        //issue2: just pressing enter, undefined behaviour.
        printf("i read this: %s\n",input);

        token = strtok(input, delim);

        int i = 0;
        argv[i] = token;
        i++;
        while (token != NULL)
        {
            printf("prev token: %s\n",token);

            token = strtok(NULL, delim); //need this NULL call to keep going, according to strtok stackoverflow

            argv[i] = token;
            printf("current i=%d token=%s\n", i, token); //issue here
            i++;
            
        }
        argv[i] = NULL;

        //execvp cant handle cd.

        if (strcmp(argv[0],"cd")==0)
        {
            //char address[1024] = "/";
            //strcat(address,argv[1]);
            //strcpy(argv[1],address);
            chdir(argv[1]);
            printf("chdir with %s\n",argv[1]);
        }
        else if(strcmp(argv[0],"exit")==0)
        {
            return 0;
        }
        else
        {
            //fork, execvp, wait
            int c_pid = 0;
            c_pid = fork();
            if (c_pid == 0) //child
            {
                printf("this is the child\n");

                int iserr = 0;
                iserr = execvp(argv[0],argv);

                if (iserr == -1) 
                {
                    perror("problem with execvp, argv[]\n");
                    printf("current argv[0]: %s\n",argv[0]);
                }  
            }
            else if (c_pid == -1)
            {
                perror("error with child process creation!\n");
            }
            else //parent
            {
                printf("in the parent, PID = %d\n",c_pid);
                int childstatus;
                wait(&childstatus);
            }
        }
    }

    return 0;
}