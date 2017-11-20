/*************************************************************************
 *                            Cliente ATM                                *
 * Authors:                                                              *
 * Edwar Yepez              12-10855                                     *
 * Maria Fernanda Machado   13-10780                                     *
 * Veronica Mazutiel        13-10853                                     *
 *                                                                       *
 *************************************************************************/

// Libraries imports
#include <stdio.h>
#include <string.h>          //strlen
#include <stdlib.h>          //strlen
#include <sys/socket.h>
#include <arpa/inet.h>       //inet_addr
#include <unistd.h>          //write
#include <pthread.h>         //for threading, link with lpthread
#include <regex.h>           //for catch patterns in message of ATM's
#include <time.h>             //for timestamps in binnacles entries
#include <getopt.h>
#include <sys/poll.h>
/* Declaracion de funcion que maneja conexiones del servidor con un ATM */
void *connection_handler(void *);

/* Definicion de funcion que vacia bufer de memoria donde se guardan los mensajes */
#define CLEAR(x) memset(x, '\0', 4096);

FILE *binnacle_fd; // Guardara el archivo de entrada
char *binnacle = ""; // Guadara el String que guarda el archivo de entrada
int serial = 0; // Irá guardando el serial de cada mensaje recibido por el SVR
/* Se declara un semáforo mutex para sincronizar la escritura en el archivo de la bitácora.*/
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 
/* Se definen los patrones que se deben reconocer */
char *patterns[14] = {"Communication Online", "Communication Offline", 
                      "Communication error", "Low Cash alert", 
                      "Running Out of notes in cassete", "empty", 
                      "Service mode entered", "Service mode left",
                      "device did not answer as expected",
                      "The protocol was cancelled", "Low Paper warning",
                      "Printer Error", "Paper-out condition", "N/A"};


int main(int argc, char *argv[]) {
    /* Enteros que denotan descriptores de sockets y variables para determinar datos
    de la línea de comandos */
    int socket_desc, client_sock, c, *new_sock, opt;
    struct sockaddr_in server, client; // Estructuras necesarias para hacer la conexión con el socket
    int svr_port = -1; // Guardará el puerto de conexión, recibido por la línea de comandos
    int error = 0; // Entero para designar cuando ocurra un error en la entrada

    /* Obtencion de parametros de entrada. Necesita especificarse el puerto
     * por donde estara escuchando el servidor y la bitacora donde se estaran
     * guardando las operaciones bancarias enviadas por los ATM.
     */
    while( ( opt = getopt( argc, argv, "l: b:" ) ) != -1 ) {
        switch(opt) {
            case 'b':
                binnacle = optarg;
                break;
            case 'l':
                svr_port = atoi(optarg);
                break;
            case '?':
                error++;
        }
    }

    /* Si falta uno de los argumentos requeridos, error. */
    if (svr_port == -1 || strcmp(binnacle, "") == 0) error++;

    /* En caso de error durante la recepcion de argumentos, se cierra el programa */
    if (error) {
        perror("\n\tERROR: el formato valido es: "
               "./svr_c -d <nombre_módulo_central> -l <puerto_svr_s> "
               "-b <archivo_bitácora>"
               "\n\tTodos los parametros entre [] son opcionales.\n");
        exit(0);
    }

    /* Se crea un nuevo socket para la conexion                   
     * Si Ocurre un error durante la creacion del mismo se aborta 
     * la ejecucion del programa */
    if ( (socket_desc = socket(AF_INET , SOCK_STREAM , 0)) == -1 ) {
        perror("Ocurrio un error durante la creacion del socket\n");
        exit(0);
    }

    puts("Se creo el socket satisfactoriamente...\n");
     
    /* Se prepara la direccion de entrada del socket */
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons( svr_port );
    
    /* Se enlaza el socket a la direccion de entrada 
     * en caso de qie no se logre enlazar, se aborta la ejecucion del programa */
    if (bind(socket_desc,(struct sockaddr *) &server , sizeof(server)) < 0) {
        perror("El enlace del socket con la direccion de entrada fallo. \n");
        exit(0);
    }

    puts("Se enlazo el socket a la direccion de entrada satisfactoriamente...\n");

    /* Empieza a escuchar el socket por nuevas conexiones */
    listen(socket_desc, 1000);
     
    puts("Esperando por nuevos pedidos de conexion... \n");
    c = sizeof(struct sockaddr_in);

    /* El cliente acepta los pedidos de nuevas conexiones y por cada pedido aceptado
     * crea un nuevo hilo encargado de manejar dicha conexion */
    while ( (client_sock = accept(socket_desc, (struct sockaddr *) &client, (socklen_t *) &c)) ) {
        puts("Conexion aceptada...\n");
    
        pthread_t sniffer_thread; // Guarda el nuevo hilo que manejará la conexión
        new_sock = malloc(1);
        *new_sock = client_sock; // Apunta al socket con el cual se aceptó la solicitud.
        
        /* Se crea un hilo que estara conectado al ATM al que se acaba de aceptar la conexion 
         * en caso de error se aborta la ejecucion del programa */ 
        if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void *) new_sock) < 0) {
            perror("Fallo la creacion de nuevo hilo de conexion con ATM. \n");
            exit(0);
        }
        
        /* Espera que los hilos creados terminen su ejecucion para terminar el */ 
        // pthread_join( sniffer_thread , NULL);
        puts("Manejador asignado a ATM...\n");
    }
    
    /* Si el descriptor del socket quedo como -1 siginica que fallo el accept.
     * En dicho caso, se aborta la ejecucion del programa */
    if (client_sock < 0) {
        perror("Fallo aceptar la conexion con ATM.\n");
        exit(0);
    }

    /* Si el archivo de la bitacora sigue abierto, se cierra antes de terminar
     * la ejecucion del programa */
    fclose(binnacle_fd);
     
    return 0;
}
/* Recibe un mensaje en message y un patrón en pattern, que si hece match
*  con el mansaje entonces devuelve 1, de lo contrario devuelve 0*/
int verify_match(char *message, char *pattern) {
    regex_t regex; //Se declara una expresión regular
    int reti; // Entero que guardará la compilación de la expresión regulares y su ejecución
    char msgbuf[100];

    /* Se compila la regex, en caso de error, aborta la ejecucion del programa */ 
    reti = regcomp(&regex, pattern, 0);
    if (reti) {
        perror("No se pudo compilar la expresion regular\n");
        exit(0);
    }

    /* Ejecuta expresion regular */
    reti = regexec(&regex, message, 0, NULL, 0);
    if (!reti) {
        regfree(&regex);
        return 1;
    }
    else if (reti == REG_NOMATCH) {
        regfree(&regex);
        return 0;
    }
    else {
        regerror(reti, &regex, msgbuf, sizeof(msgbuf));
        fprintf(stderr, "Fallo el match con expresion regular: %s\n", msgbuf);
        exit(0);
    }
}

/* Devuelve el id del patron de advertencia que contiene un mensaje. En caso
 * de que no contenga ninguno, devuelve -1 */ 
int catch_pattern(char *message) {
    /* Se declaram un arreglo con los posibles patrones que pueden hacer match*/
    char *patterns[13] = {".*Communication Online", ".*Communication Offline", 
                          ".*Communication error", ".*Low Cash alert", 
                          ".*Running Out of notes in cassette", ".*empty", 
                          ".*Service mode entered", ".*Service mode left",
                          ".*device did not answer as expected",
                          ".*The protocol was cancelled", ".*Low Paper warning",
                          ".*Printer Error", ".*Paper-out condition"};
    /* Se busca el mensaje en el arreglo patterns*/
    for (int i = 0; i < 13; i++) {
        if ( verify_match(message, patterns[i]) ) {
            return i;
        }
    }
    return -1;
}
/* Función que escribe en pantalla y en el archivo de bitácora los datos del mensaje recibido*/
void write_entry(int pattern_id, char *client_message, char *ip) {
    char buffer[4096], message[4096];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    CLEAR(message);

    serial++; // Se aumenta el serial para que varía dependiendo del orden de llegada de los mensajes

    fputs("\n", binnacle_fd);

    strcat(message, "SERIAL: ");
    sprintf(buffer, "%d", serial);
    strcat(message, buffer);
    CLEAR(buffer);

    strcat(message, "\nFECHA: ");
    sprintf(buffer, "%d-%d-%d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
    strcat(message, buffer);
    CLEAR(buffer);
    
    strcat(message, "\nHORA: ");
    sprintf(buffer, "%d:%d:%d", tm.tm_hour, tm.tm_min, tm.tm_sec);
    strcat(message, buffer);
    CLEAR(buffer);
    
    strcat(message, "\nATM ID: ");
    strcat(message, ip);

    strcat(message, "\nEVENT ID: ");
    sprintf(buffer, "%d", pattern_id);
    strcat(message, buffer);
    CLEAR(buffer);

    strcat(message, "\nPATTERN: ");
    strcat(message, patterns[pattern_id]);

    strcat(message, "\nMESSAGE: ");
    strcat(message, client_message);

    fputs(message, binnacle_fd);
    fputs("\n", binnacle_fd);

    puts("");
    puts("Nueva transaccion registrada: ");
    puts("-------------------------------------------");
    printf("%s", message);
    puts("\n-------------------------------------------");
    puts("");

    if (pattern_id == 1 || pattern_id == 2) {
        strcat(buffer, "echo '");
        strcat(buffer, message);
        strcat(buffer, "' | mail -s 'ALERTA DE ATM' edwarypz@gmail.com");
        system(buffer);
    }

    CLEAR(message);
}

/* Maneeja la conexion para cada cliente */
void *connection_handler(void *socket_desc) {
    /* Se definen las variables necesarias para la recepción de los mensajes del ATM*/
    int sock = *(int *) socket_desc;
    int read_size, port, pattern_id, ret;
    char client_message[4096];
    struct sockaddr_storage addr;
    char ipstr[INET6_ADDRSTRLEN];
    socklen_t len;
    struct pollfd fd;

    /* Obtiene el ip del ATM con el que esta conectado */
    len = sizeof addr;
    getpeername(sock, (struct sockaddr *) &addr, &len);

    struct sockaddr_in *s = (struct sockaddr_in *) &addr;
    port = ntohs(s->sin_port);
    inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);

    printf("Conectado al ATM con id: %s\n", ipstr);
    /* Primera escritura en la bitácora para alarmar la conexión*/
    pthread_mutex_lock(&mutex); 
    binnacle_fd = fopen(binnacle, "a+");
    write_entry(0, "Communication Online.", ipstr);
    fclose(binnacle_fd);
    pthread_mutex_unlock(&mutex);

    fd.fd = sock;
    fd.events = POLLIN;
    /* Entero que si vale 0 significa que se cerro la conexión y no se deben esperar más mensajes*/
    int conectado = 1; 

    while (conectado) {
        /* Espera por un mensaje de recv durante un tiempo establecido en milisegundos*/
        ret = poll(&fd, 1, 60000);
        switch (ret) {
            /* Si ret es -1 ocurrio un error en la funcion */
            case -1:
                perror("OCURRIO ERROR CON FUNCION poll()");
                exit(1);
            /* Si ret es 0 es porque pasó el tiempo de espera */
            case 0:
                pthread_mutex_lock(&mutex);

                if ((binnacle_fd = fopen(binnacle, "a+")) < 0) {
                    perror("ERROR: No se pudo abrir el archivo de la bitacora");
                }

                write_entry(2, patterns[2], ipstr);
                fclose(binnacle_fd);
                pthread_mutex_unlock(&mutex);
                break;
            /* De lo contrario se recibió un nuevo mensaje*/
            default:
                /* Guarda un descriptor que indica el estado del mensaje leído*/
                read_size = recv(sock, client_message, 4096, 0);
                /* Si read_size es 0 significa que mayor a 0 se recibió un mensaje,
                *  por lo que se abre el archivo y se escribe el nuevo mensaje usando un semaforo*/ 
                if (read_size > 0) {
                    pattern_id = catch_pattern(client_message);

                    pthread_mutex_lock(&mutex);

                    /* Una vez recibido el mensaje, abre la bitacora para transcribir la nueva
                     * entrada. */
                    if ((binnacle_fd = fopen(binnacle, "a+")) < 0) {
                        perror("ERROR: No se pudo abrir el archivo de la bitacora");
                    }

                    if (pattern_id == -1) {
                        write_entry(13, client_message, ipstr);
                    }
                    else {
                        write_entry(pattern_id, client_message, ipstr);
                    }
                    
                    CLEAR(client_message);
                    fclose(binnacle_fd);

                    pthread_mutex_unlock(&mutex);
                }
                /* Si read_size es 0 significa que se cerro la conexión por parte del ATM*/
                else if (read_size == 0) {
                    /* Se escribe en la bitácora el mensaje de desconexión*/
                    pthread_mutex_lock(&mutex);
                    binnacle_fd = fopen(binnacle, "a+");
                    write_entry(1, "Communication Offline.", ipstr);
                    fclose(binnacle_fd);
                    pthread_mutex_unlock(&mutex);
                    printf("\nATM de id: %s desconectado.\n", ipstr);
                    fflush(stdout);
                    conectado = 0;
                }
                // Ocurrió un error
                else if (read_size == -1) {
                    perror("Fallo en funcion de recv.");
                    exit(0);
                }
        }
    }    
         
    /* Se libera el descriptor del socket */
    free(socket_desc);
     
    return 0;
}
