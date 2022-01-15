#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "shell.h"
#include <signal.h>
#include <fcntl.h>

#define MAXW 1000
#define MAXCOM 100

static void CtrlChandler(int sig)
{

}
static void CtrlZhandler(int sig)
{


}
static void CtrlBarhandler(int sig)
{

}

int main(int argc, char **argv)
{
    

    while (1)
    {

        
        signal(SIGINT, CtrlChandler);
        signal(SIGTSTP, CtrlZhandler);
        signal(SIGQUIT, CtrlBarhandler);
        

        char inputString[MAXCOM], *parsedArgs[MAXW], *parsedArgsPiped1[MAXCOM], *parsedArgsPiped2[MAXCOM], *io[MAXCOM];
        int flag = 0;
        int background = 0;

        if (argc < 2)
        {
            takeInput(inputString);
        }

       


        flag = processString(inputString, parsedArgs, argc, argv, parsedArgsPiped1, parsedArgsPiped2, io);

        background = isBackground(parsedArgs);



        argc = 0;

        switch (flag)
        {
        case 0:
            builtInCommands(parsedArgs);
            break;
        case 1:

            systemCom(parsedArgs, background);
            break;
        case 2:;
            FILE *batchFile = fopen("batchfile.sh", "r");
            batchFileCommands(batchFile);
            return 0;
        case 3:
            execArgsPiped(parsedArgs, parsedArgsPiped1, parsedArgsPiped2);
            break;
        case 4:
            execInputR(parsedArgs, io);
            break;
        case 5:
            execOutputR(parsedArgs, io);
            break;
        default:
            break;
        }

        
    }

    return 0;
}
