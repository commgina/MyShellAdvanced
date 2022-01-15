#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <limits.h>
#include "shell.h"
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#define MAXCOMMANDS 100
#define MAXWORDS 1000

int jobCounter = 0;
int cantPipes = 0;

struct sigaction sigchld_action = {
    .sa_handler = SIG_DFL,
    .sa_flags = SA_NOCLDWAIT};

/**Codigos ansi de escape
 * \033[H mueve el cursor al comienzo
 * \033[J limpia todo lo que hay desde el cursor hasta el final de la pantalla
 * */

#define clear() printf("\033[H\033[J");
// funcion para tomar el input del usuario
int takeInput(char *str)
{

    char hostname[HOST_NAME_MAX + 1];
    gethostname(hostname, HOST_NAME_MAX + 1);
    char *input;
    printf("\n%s@%s", getenv("USER"), hostname);
    input = readline(":~$ ");
    if (strlen(input) != 0)
    {
        add_history(input); // agrego el input al historial
        strcpy(str, input); // copio el input a str
        return 0;
    }
    else
    {
        return 1;
    }
}

int processString(char *str, char **parsed, int argc, char **argv, char **parsedPipe1, char **parsedPipe2, char **io)
{



    cantPipes = contadorPipes(str);
    char straux[MAXCOMMANDS];
    strcpy(straux, str);
    char *strpiped[3];
    int piped = 0;
    piped = parsePipe(str, strpiped);
    char *strRedirect1[2], *strRedirect2[2];
    int ir = inputRedirection(straux, strRedirect1);
    int or = outputRedirection(straux, strRedirect2);

    if (ir)
    {
        
        parseSpace(strRedirect1[0], parsed);
        io[0] = strRedirect1[1];
        if(strcmp(parsed[0], "echo") == 0){
            
            printEcho("",parsed,NULL);
            return 6;

        }else{

            return 4;

        }
   
    }
    else if (or)
    {

        
        parseSpace(strRedirect2[0], parsed);
        io[0] = strRedirect2[1];
        if(strcmp(parsed[0], "echo") == 0){
            printEcho("",parsed,io[0]);
            return 6;

        }else{

            return 5;

        }
        
    }

    if (piped)
    {

        parseSpace(strpiped[0], parsed);
        parseSpace(strpiped[1], parsedPipe1);
        if (cantPipes == 2)
        {
            parseSpace(strpiped[2], parsedPipe2);
        }

        return 3;
    }

    if (argc > 1)
    {
        int i;
        /** Argv ya esta parseado, solo debo eliminar el primer argumento que es el nombre del programa */
        for (i = 0; i < argc; i++)
        {
            parsed[i] = argv[i + 1];
        }
    }
    else
    {

        parseSpace(str, parsed);
    }

    /**Una vez hice el parse, busco si se trata de un comando interno,
     * la llamada a un programa externo,
     * la ejecucion de un batchfile
     * la ejecucion de un programa externo en background */
    if (strcmp(parsed[0], "clr") == 0 || strcmp(parsed[0], "cd") == 0 || strcmp(parsed[0], "quit") == 0 || strcmp(parsed[0], "echo") == 0)
    {
        return 0;
    }
    else if (strcmp(parsed[0], "batchfile") == 0)
    {
        return 2;
    }
    else
    {
        /** Aca entrara si se trata de un programa externo, pero debo checkear si sera ejecutado en background */
        return 1;
    }
}

int isForeground()
{
    pid_t pid = tcgetpgrp(STDIN_FILENO);
    if (pid == -1 /* piped */ || pid == getpgrp() /* foreground */)
        return 1;
    /* otherwise background */
    return 0;
}

int isBackground(char **parsed)
{

    int bg = 0;

    for (int i = 0; parsed[i] != NULL; i++)
    {

        if (strcmp(parsed[i], "&") == 0)
        {
            parsed[i] = NULL;
            bg = 1;
            break;
        }
    }

    return bg;
}

int outputRedirection(char *str, char **strRed)
{
    int i;
    for (i = 0; i < 2; i++)
    {
        strRed[i] = strsep(&str, ">");
        if (strRed[i] == NULL)
            break;
    }

    if (strRed[1] == NULL)
        return 0;
    else
    {
        return 1;
    }
}

int inputRedirection(char *str, char **strRed)
{
    int i;
    for (i = 0; i < 2; i++)
    {
        strRed[i] = strsep(&str, "<");
        if (strRed[i] == NULL)
            break;
    }

    if (strRed[1] == NULL)
        return 0;
    else
    {
        return 1;
    }
}

void parseSpace(char *str, char **parsed)
{
    int i;

    for (i = 0; i < MAXCOMMANDS; i++)
    {
        parsed[i] = strsep(&str, " ");

        if (parsed[i] == NULL)
            break;
        if (strlen(parsed[i]) == 0)
            i--;
    }
}

void execInputR(char **commands, char **io)
{

    pid_t pid;
    pid = fork();

    if (pid == -1)
    {

        perror("Error en el fork\n");
    }
    if (pid == 0)
    {

        int fd0 = open(io[0], O_RDONLY, 0);
        dup2(fd0, STDIN_FILENO);
        close(fd0);
        if (execvp(commands[0], commands) < 0)
        {
            printf("\nNo se pudo ejecutar el comando 1..");
        }

        exit(0);
    }

    wait(NULL);
}

void execOutputR(char **commands, char **io)
{

    pid_t pid;
    pid = fork();

    if (pid == -1)
    {

        perror("Error en el fork\n");
    }
    if (pid == 0)
    {

        int fd0 = creat(io[0], 0644);
        dup2(fd0, STDOUT_FILENO);
        close(fd0);
        if (execvp(commands[0], commands) < 0)
        {
            printf("\nNo se pudo ejecutar el comando 1..");
        }

        exit(0);
    }

    wait(NULL);
}

int contadorPipes(char *inputString)
{

    int cantPipes = 0;

    for (int i = 0; inputString[i] != '\0'; i++)
    {

        if (inputString[i] == '|')
        {
            cantPipes++;
        }
    }

    return cantPipes;
}

void execArgsPiped(char **parsed0, char **parsedpipe1, char **parsedpipe2)
{
    pid_t pid;
    int pipe1[2];
    int pipe2[2];
    if (pipe(pipe1) != 0 || pipe(pipe2) != 0)
    {
        perror("Fallo en alguna pipe\n");
        return;
    }

    pid = fork();

    if (pid < 0)
    {

        perror("Fallo en el fork\n");
        return;
    }

    if (pid == 0)
    {

        dup2(pipe1[1], 1);
        close(pipe1[0]);
        close(pipe1[1]);
        if (cantPipes == 2)
        {
            close(pipe2[0]);
            close(pipe2[1]);
        }
        
        if (execvp(parsed0[0], parsed0) < 0)
        {
            printf("\nNo se pudo ejecutar el comando 1..");
        }
        exit(0);
    }

    fflush(stdout);

    pid_t pid2 = fork();
    if (pid2 < 0)
    {

        perror("Fallo en el fork\n");
        return;
    }

    if (pid2 == 0)
    {

        dup2(pipe1[0], 0);
        close(pipe1[0]);
        close(pipe1[1]);
        if (cantPipes == 2)
        {

            dup2(pipe2[1], 1);
            close(pipe2[0]);
            close(pipe2[1]);
        }

        if (execvp(parsedpipe1[0], parsedpipe1) < 0)
        {

            printf("\nNo se pudo ejecutar el comando 2..");
        }
        exit(0);
    }

    fflush(stdout);

    if (cantPipes == 2)
    {
        pid_t pid3 = fork();
        if (pid3 < 0)
        {
            perror("Fallo en el fork");
            return;
        }

        if (pid3 == 0)
        {

            dup2(pipe2[0], 0);
            close(pipe1[0]);
            close(pipe1[1]);
            close(pipe2[0]);
            close(pipe2[1]);
            if (execvp(parsedpipe2[0], parsedpipe2) < 0)
            {

                printf("\nNo se pudo ejecutar el comando 2..");
            }
            exit(0);
        }

        fflush(stdout);
    }

    close(pipe1[0]);
    close(pipe1[1]);
    close(pipe2[0]);
    close(pipe2[1]);

    pid_t zombie;
    int status;
    while ((zombie = wait(&status)) > 0)
        ;
}

int parsePipe(char *str, char **strpiped)
{
    int i;
    for (i = 0; i < 3; i++)
    {
        strpiped[i] = strsep(&str, "|");
        if (strpiped[i] == NULL)
            break;
    }

    if (strpiped[1] == NULL)
        return 0;
    else
    {
        return 1;
    }
}

// funcion para imprimir el directorio actual
void printCurrentDirectory()
{
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd)); // Get the pathname of the current working directory and put it in SIZE bytes of BUF, es de unistd
    printf("\nEL directorio actual es: %s", cwd);
}
// funcion que ejecuta comandos del sistema
void systemCom(char **comandoParseado, int background)
{

    // hago fork de un child

    pid_t pid = fork();
    int status = 0;

    // int status=0;

    if (pid == -1)
    {
        perror("fork issue\n");
        exit(0);
    }
    else if (pid == 0)
    {
        if (background != 0)
        {
            signal(SIGINT, SIG_IGN);
            signal(SIGTSTP, SIG_IGN);
            signal(SIGQUIT, SIG_IGN);
        }

        if (execvp(comandoParseado[0], comandoParseado) < 0)
        { // si devuelve -1 el execvp es porque hubo un fallo y ademas establece la variable errno

            printf("\nNo se pudo ejecutar el comando...");
        }

        // el primer argumento es el nombre del archivo o ruta del nuevo archivo a cargar y el segundo es el array de argumentos

        exit(errno);
    }
    else
    {

        if (background == 0)
        {

            waitpid(pid, &status, WUNTRACED);
        }
        else
        {

            jobCounter++;
            printf("[%d] [%d]", jobCounter, pid);
            sigaction(SIGCHLD, &sigchld_action, NULL);
        }
    }

    if (waitpid(0, NULL, WNOHANG) != 0)
    {
        // espero que todos los hijos mueran para resetear el contador, al igual que bash
        jobCounter = 0;
    }
}

void changeDir(char *parsed)
{

    if (strcmp(parsed, "-") == 0)
    {
        chdir(getenv("OLDPWD"));
        setenv("PWD", getenv("OLDPWD"), 1);
    }
    else
    {
        if (chdir(parsed) == 0)
        {
            setenv("OLDPWD", getenv("PWD"), 1);
            setenv("PWD", parsed, 1);
            printf("Directorio cambiado. El directorio actual es %s\n", getenv("PWD"));
        }
        else if (chdir(parsed) == -1)
        {

            // si el directorio no existe
            if (errno == ENOENT)
            {
                perror("No existe el archivo o directorio solicitado.");
            }
        }
    }
}

void printEcho(char *parsed, char **parsedArray, char *fileName)
{

    char forPrint[MAXWORDS] = "";
    if (parsed[0] == '$')
    {
        char *substring = parsed + 1;
        printf("%s\n", getenv(substring));
    }
    else if(parsedArray[1][0] == '$'){
        char *substring = parsedArray[1] + 1;
        //printf("%s\n", getenv(substring));
        strcat(forPrint,getenv(substring));

    }
    else
    {

        
        for (int i = 1; i < MAXCOMMANDS; i++)
        {

            if (parsedArray[i] == NULL)
            {

                break;
            }
            strcat(forPrint, parsedArray[i]);
            strcat(forPrint, " ");
        }
        printf("%s\n", forPrint);
    }

    if(fileName != NULL){
        FILE* newFile = fopen(fileName,"w");
        fputs(forPrint,newFile);
        fclose(newFile);
    }
}

void batchFileCommands(FILE *bf)
{

    char linea[MAXWORDS], *parsedArgs[MAXCOMMANDS];
    int flag = 0;
    int background = 0;

    while (fgets(linea, sizeof(linea), bf))
    {

        flag = processString(linea, parsedArgs, 0, NULL, NULL, NULL, NULL);
        background = isBackground(parsedArgs);

        if (flag == 0)
        {
            builtInCommands(parsedArgs);
        }
        else if (flag == 1)
        {
            systemCom(parsedArgs, background);
        }
    }
}
// funcion para ejecutar comandos internos
int builtInCommands(char **parsed)
{

    int cantComandos = 4, i, numeroSwitch = 0;
    char *listaComandos[cantComandos];
    // char* usuario;

    listaComandos[0] = "cd";
    listaComandos[1] = "clr";
    listaComandos[2] = "echo";
    listaComandos[3] = "quit";

    for (i = 0; i < cantComandos; i++)
    {

        if (strcmp(parsed[0], listaComandos[i]) == 0)
        {

            numeroSwitch = i + 1;
            break;
        }
    }

    switch (numeroSwitch)
    {
    case 1:
        if (parsed[1] != NULL)
        {

            changeDir(parsed[1]);
        }
        else
        {
            printCurrentDirectory();
        }
        return 1;
    case 2:
        clear();
        return 1;
    case 3:
        if (parsed[1] != NULL)
        {
            printEcho(parsed[1], parsed,NULL);
        }
        return 1;
    case 4:
        exit(0);
    default:
        break;
    }

    return 0;
}
