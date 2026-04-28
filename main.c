#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

int in(char *argv[], int len, char *chr, char *chr2)
{
    //printf("in()\n");
    //returns 1 if chr in argv, 0 otherwise
    for (int i = 0; i < len; i++)
    {
        if (argv[i] != NULL && (strcmp(argv[i],chr)==0 || strcmp(argv[i], chr2)==0))
            return 1;
    }
    return 0;
}

int locate(char *argv[], int len, char *chr, char *chr2) //check: * chr
{
    //printf("locate()\n");
    //returns location if chr in argv, -1 otherwise
    for (int i = 0; i < len; i++)
    {
        if (argv[i] != NULL && (strcmp(argv[i],chr)==0 || strcmp(argv[i], chr2)==0))
            return i;
    }
    return -1;
}

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
            //printf("prev token: %s\n",token);

            token = strtok(NULL, delim); //need this NULL call to keep going, according to strtok stackoverflow

            argv[i] = token;
            printf("current i=%d token=%s\n", i, token);
            i++;
            
        }
        argv[i] = NULL;
        int len = i-1; //len is the first invalid argv index, also the number of tokens
        //printf("len is %d\n",len);

        //execvp cant handle cd.

        if(strcmp(argv[0],"exit")==0)
        {
            return 0;
        }
        else if(in(argv,len,"|","|"))
        {
            printf("pipe (|) detected\n");

            int loc = locate(argv,len,"|","|");

            int err=0;
            int p[2];
            err=pipe(p);
            if (err==-1)
                perror("problem with pipe()\n");

            int c_pid = 0;
            c_pid = fork();

            if (c_pid == 0) //child1
            {
                //writes
                printf("in the child 1\n");
                close(p[0]);
                int res1;
                res1=dup2(p[1],STDOUT_FILENO);
                if (res1==-1)
                    perror("error with dup2");

                argv[loc]=NULL;
                close(p[1]);
                int call=execvp(argv[0],argv);
                if (call==-1)
                    perror("error with execvp in dup2");

            }
            else //parent
            {
                printf("in the parent\n");

                int c_pid2 = 0;
                c_pid2 = fork();
                //what to close?

                if (c_pid2 == 0) //child2
                {
                    //reads
                    close(p[1]);
                    printf("in the child2\n");
                    int res2;
                    res2=dup2(p[0],STDIN_FILENO);
                    //error handling
                    close(p[0]);
                    execvp(argv[loc+1],&argv[loc+1]);

                }
                close(p[0]);
                close(p[1]);
                //where to close?
                int childstatus;
                wait(&childstatus);
                int childstatus2;
                wait(&childstatus2);
            }
        }
        else if(in(argv,i,"<",">"))
        {
            printf("io redirection (<,>) detected\n");
            //io redirection
        }
        else if(strcmp(argv[0],"sleep")==0)
        {
            //sleep
        }
        else if (strcmp(argv[0],"cd")==0)
        {
            //char address[1024] = "/";
            //strcat(address,argv[1]);
            //strcpy(argv[1],address);
            chdir(argv[1]);
            printf("chdir with %s\n",argv[1]);
        }
        else //other calls
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