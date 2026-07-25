#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>

void sig_handler(int sig){
    int status;
    int res=1;
    while(res>0){
        res=waitpid(-1, &status, WNOHANG);
    }
}

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
    int bgflag=1;

    printf("hello. welcome to the shell\n");

    struct sigaction sa;
    sa.sa_handler=&sig_handler;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("error, sigaction");
        return 1;
    }

    struct sigaction sa2;
    sa2.sa_handler = SIG_IGN; 
    sa2.sa_flags = 0;
    sigemptyset(&sa2.sa_mask);

    if (sigaction(SIGINT, &sa2, NULL) == -1) {
        perror("error, sigaction");
        return 1;
    }

    struct sigaction sa3;
    sa3.sa_handler = SIG_IGN; 
    sa3.sa_flags = 0;
    sigemptyset(&sa3.sa_mask);

    if (sigaction(SIGTSTP, &sa3, NULL) == -1) {
        perror("error, sigaction");
        return 1;
    }

    struct sigaction sadef;
    sadef.sa_handler = SIG_DFL; 
    sadef.sa_flags = 0;
    sigemptyset(&sadef.sa_mask);

    while (fgets(input, sizeof(input), stdin) != NULL)
    {
        bgflag=1;
        input[strcspn(input, "\n")] = '\0';
        //issue1: what if someone wants to put multiple lines?
        //issue2: just pressing enter, undefined behaviour.
        //issue3: what if there are multiple |'s or <'s
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
        if (in(argv,len,"&","&")) //background process
        {
            bgflag=0;
            int loc_amp = locate(argv,len,"&","&");
            argv[loc_amp] = NULL;
        }
        if(strcmp(argv[0],"exit")==0) //exit
        {
            return 0;
        }
        else if(in(argv,len,"|","|")) //pipe
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
                if (sigaction(SIGTSTP, &sadef, NULL) == -1) {
                    perror("error, sigaction");
                    return 1;
                }
                if (sigaction(SIGINT, &sadef, NULL) == -1) {
                    perror("error, sigaction");
                    return 1;
                }

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
                    if (sigaction(SIGTSTP, &sadef, NULL) == -1) {
                    perror("error, sigaction");
                    return 1;
                    }
                    if (sigaction(SIGINT, &sadef, NULL) == -1) {
                        perror("error, sigaction");
                        return 1;
                    }
                    //reads
                    close(p[1]);
                    printf("in the child2\n");
                    int res2;
                    res2=dup2(p[0],STDIN_FILENO);
                    if (res2==-1)
                        perror("error with dup2");
                    close(p[0]);
                    execvp(argv[loc+1],&argv[loc+1]);

                }
                close(p[0]);
                close(p[1]);
                //where to close?
                if (bgflag)
                {
                    int childstatus;
                    wait(&childstatus);
                    int childstatus2;
                    wait(&childstatus2);
                }
            }
        }
        else if(in(argv,len,"<",">")) // io redirection
        {
            printf("io redirection (<,>) detected\n");
            // command <> file

            int loc = locate(argv,len,"<",">");
            int flag = (strcmp(argv[loc], "<") == 0);


            int c_pid = 0;
            c_pid = fork();

            if (c_pid == 0) //child1
            {
                if (sigaction(SIGTSTP, &sadef, NULL) == -1) {
                    perror("error, sigaction");
                    return 1;
                }
                if (sigaction(SIGINT, &sadef, NULL) == -1) {
                    perror("error, sigaction");
                    return 1;
                }
                //writes
                printf("in the child 1\n");

                int res1;
                printf("%s\n",argv[len+1]);
                int fd;
                if (flag)
                {
                    fd = open(argv[loc+1], O_RDONLY); //which modes?
                    res1=dup2(fd,STDIN_FILENO);
                }
                else
                {
                    fd = open(argv[loc+1], O_WRONLY | O_CREAT | O_TRUNC, 0644); //which modes?
                    res1=dup2(fd,STDOUT_FILENO);
                }
                if (res1==-1)
                    perror("error with dup2");

                argv[loc]=NULL;
                close(fd);
                int call=execvp(argv[0],argv); //is this how to exec until argv[loc]?
                if (call==-1)
                    perror("error with execvp in dup2");
            }
            else //parent
            {
                printf("in the parent\n");
                if (bgflag)
                {
                    int childstatus;
                    wait(&childstatus);
                }

            }

        }
        else if (in(argv,len,"cd","cd")) //cd
        {
            int loc = locate(argv,len,"cd","cd");
            chdir(argv[loc+1]);
            printf("chdir with %s\n",argv[1]);
        }
        else //other calls
        {
            //fork, execvp, wait
            int c_pid = 0;
            c_pid = fork();
            if (c_pid == 0) //child
            {
                if (sigaction(SIGTSTP, &sadef, NULL) == -1) {
                    perror("error, sigaction");
                    return 1;
                }
                if (sigaction(SIGINT, &sadef, NULL) == -1) {
                    perror("error, sigaction");
                    return 1;
                }
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
                if (bgflag)
                {
                    int childstatus;
                    wait(&childstatus);
                }
            }
        }
    }

    return 0;
}