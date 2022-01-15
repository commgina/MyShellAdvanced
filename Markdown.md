# Trabajo Practico 4

## Desarrollo
Para el desarrollo de este trabajo utilice lo visto en clase sobre creación de procesos mediante fork y exec, system calls, procesos zombies y las señales.
A diferencia de mis trabajos anteriores, intente modularizar el código, es decir, separar cada funcionalidad en un metodo distinto para que a la hora de corregir algún error no me llevara tanto tiempo y pudiera distinguir mas rápidamente de donde venia.

Sin duda la mayor dificultad del trabajo fue el manejo de las entradas del usuario ya que tenia dos opciones, que se ejecute el programa con argumentos o sin argumentos, por lo tanto decidi trabajar con args y argv y otro método llamado takeInput para el caso de la ejecución sin argumentos. Este metodo utiliza la libreria __readline__ de GNU.
La segunda dificultad fue evitar la creación de procesos zombies ya que no entendía bien sobre el tema.

## Explicación breve de cada método

- void parseSpace(char *str, char **parsed) : parsea la entrada del usuario guardando cada token en un array
- int takeInput(char *str): toma la entrada del usuario y muestra el command prompt
- int builtInCommands(char **parsed): método principal para la ejecucion de comandos internos, toma el segundo elemento del array de parseados para ver que funcionalidad se quiere ejecutar.
- void printEcho(char *parsed, char **parsedArray): funcion que imita el funcionamiento de echo de la terminal de linux.
- void changeDir(char *parsed) : función que cambia el directorio y las variables de entorno PWD y OLDPWD
- void systemCom(char **comandoParseado, int background): funcion que hace el fork y execvp. Se le pasa como argumento el valor que arroja isBackground y en base a eso se ejecuta un proceso en background o foreground. Posee un contador de job que se reinicia cuando todos los procesos con el mismo pid del padre terminan de ejecutarse. Mediante señales se eliminan los procesos que terminaron de ejecutarse en background.
- void printCurrentDirectory(): una funcion sencilla que imprime el directorio actual.
- int processString(char *str, char **parsed, int argc, char **argv): esta funcion procesa el input ya sea el que viene por argv o el que viene por takeinput y retorna un int dependiendo si se  trata de un comando interno o externo
- void batchFileCommands(FILE *bf): esta funcion recibe un archivo como parametro y va ejecutando linea por linea los comandos.
- int isBackground(char **parsed): funcion que detecta si hay un & como ultimo token para saber si el proceso se ejecutara en background.

# Trabajo Practico 5

¿Dónde se encuentran los pipes en el filesystem, qué atributos tienen?

Los pipes se encuentran en la RAM, son la forma mas antigua de IPC y sin unidireccionales. Tipicamente luego de crear un pipe se realiza un fork. Tienen tamaño limitado (64k en la mayoria de sistemas).
Las pipes que se encuentran en el filesystem con las Named pipes o tambien llamadas FIFOS, es un archivo similar a un pipe pero que tiene un nombre en el filesystem. Multiples procesos pueden acceder a este archivo para leer y escribir. Sin embargo este nombre solo funciona como punto de referencia para procesos que necesitan usar el nombre en el filesystem. A diferencia de las annonymous pipes, estas son bidireccionales.