#include "stdio.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <signal.h>
#define SIZE 50
#define length 510

pid_t childpid;


void handler(int signum);
void child_handler(int signum);

void error(char *msg){
    perror(msg);
    exit(1);
}
int main()
{
    //Catch signals.
    signal(SIGTSTP, handler);
    signal(SIGCHLD, child_handler);

    int argv_count = 0;       // count current commands chars only
    int total_count = 0;   // total chars for all commands
    int command_count = 0; //total commands counter.
    char user_IN[length];     // user commands input
    char copy[length];        // copy of user_IN
    char buffer[SIZE];        // path string
    int i = 0;

    getcwd(buffer, SIZE); // get the path string
    
    struct passwd *user_name;

    user_name = getpwuid(getuid()); // get the user name string

    while (strcmp("done", user_IN) != 0)
    {
        printf("%s@%s>", user_name->pw_name, buffer); // output of the shell

        fgets(user_IN, length, stdin); // get input from the user
        
        //if fgets() return NULL- continue the loop.
        if(user_IN == NULL)
            continue; 

        //delete '\n' from the string
        user_IN[strlen(user_IN) - 1] = '\0'; 

        //If user type "done"- end the shell
        if (strcmp("done", user_IN) == 0)
            break;

        // if no input but enter, repeat till get input.
        if (user_IN[0] == '\0')
            continue;

        strcpy(copy, user_IN);

        char *word = strtok(user_IN, " ");
        // if there only spaces on the string- continue the loop.
        if (word == NULL)
            continue;

        total_count++;
        command_count += strlen(copy);

        //unallowed command 'cd'
        if (strcmp(word, "cd") == 0 || strcmp(word, "cd..") == 0){
            printf("command not supported (Yet)\n");
            continue;
        }
        //Continue delayed process
        if(strcmp("bg",word) == 0){
            kill(childpid,SIGCONT);
            continue;
            }

        argv_count = 1; // reset this to 1 since its requiered only for the current commands.

        //check how many words contain the string from the user.
        while (word != NULL)
        {
            argv_count++;
            word = strtok(NULL, " ");
        }
        //array for words
        char **argv = (char **)malloc(sizeof(char *) * argv_count);
        assert(argv);

        char *word_copy = strtok(copy, " ");
        
        // fill the argv
        i = 0;
        while (word_copy != NULL)
        {
            argv[i] = (char *)malloc(sizeof(char) * strlen(word_copy));
            assert(argv[i]);
            strcpy(argv[i], word_copy);
            i++;
            word_copy = strtok(NULL, " ");
        }

        argv[i] = NULL;

        // send the son procces command to execute.
        
        if((childpid = fork()) < 0)
            error("Fork Failed!\n");

        if (childpid == 0)
        {
            //Process disconnect from parent, and wait for signal while run.
            setpgrp();
            signal(SIGTSTP, SIG_DFL);
            execvp(argv[0], argv);
            exit(-1);
        }
        //return to parent and continue the loop.
        pause();

        for (i = 0; i < argv_count; i++)
            free(argv[i]);
        free(argv);
        
    }

    //exit the program
    command_count += strlen("done"); // for "done"
    total_count++;                   // for "done"

    // print summary
    printf("Num of commands: %d\n", total_count);
    printf("Total length of all commands: %d\n", command_count);
    printf("Averege length of all commands: %f\n", (double)(command_count / total_count));
    printf("See you Next Time !\n");
    return 0;
}

//Handler for TSTP from Parent.
void handler(int signum){
    kill(childpid,SIGTSTP);
    raise(SIGCHLD);  
}
//Handler for CHLD FROM Parent.
void child_handler(int signum){
    waitpid(-1, NULL, WNOHANG);
}