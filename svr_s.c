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

FILE *binnacle_fd;
char *binnacle = "";
int serial = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
char *patterns[14] = {"Communication Online", "Communication Offline", 
                      "Communication error", "Low Cash alert", 
                      "Running Out of notes in cassete", "empty", 
                      "Service mode entered", "Service mode left",
                      "device did not answer as expected",
                      "The protocol was cancelled", "Low Paper warning",
                      "Printer Error", "Paper-out condition", "N/A"};


int main(int argc, char *argv[]) {
    int socket_desc, client_sock, c, *new_sock, opt;
    struct sockaddr_in server, client;
    int svr_port = -1;
    int error = 0;

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
        
        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = client_sock;
        
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

int verify_match(char *message, char *pattern) {
    regex_t regex;
    int reti;
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
    char *patterns[13] = {".*Communication Online", ".*Communication Offline", 
                          ".*Communication error", ".*Low Cash alert", 
                          ".*Running Out of notes in cassette", ".*empty", 
                          ".*Service mode entered", ".*Service mode left",
                          ".*device did not answer as expected",
                          ".*The protocol was cancelled", ".*Low Paper warning",
                          ".*Printer Error", ".*Paper-out condition"};
    
    for (int i = 0; i < 12; i++) {
        if ( verify_match(message, patterns[i]) ) {
            return i;
        }
    }
    return -1;
}

void write_entry(int pattern_id, char *client_message, char *ip) {
    char buffer[4096], message[4096];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    CLEAR(message);

    serial++;

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

    CLEAR(message);
}

/* Maneeja la conexion para cada cliente */
void *connection_handler(void *socket_desc) {
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

    pthread_mutex_lock(&mutex);
    binnacle_fd = fopen(binnacle, "a+");
    write_entry(0, "Communication Online.", ipstr);
    fclose(binnacle_fd);
    pthread_mutex_unlock(&mutex);

    fd.fd = sock;
    fd.events = POLLIN;
     
    int conectado = 1;

    while (conectado) {

        ret = poll(&fd, 1, 4000);
        switch (ret) {
            case -1:
                perror("OCURRIO ERROR CON FUNCION poll()");
                exit(1);
            case 0:
                pthread_mutex_lock(&mutex);

                if ((binnacle_fd = fopen(binnacle, "a+")) < 0) {
                    perror("ERROR: No se pudo abrir el archivo de la bitacora");
                }

                write_entry(2, patterns[2], ipstr);
                fclose(binnacle_fd);
                pthread_mutex_unlock(&mutex);
                break;
            default:
                read_size = recv(sock, client_message, 4096, 0);
                if (read_size > 0) {
                    pattern_id = catch_pattern(client_message);

                    pthread_mutex_lock(&mutex);

                    /* Una vez recibido el mensaje, abre la bitacora para transcribir la nueva
                     * entrada. */
                    if ((binnacle_fd = fopen(binnacle, "a+")) < 0) {
                        perror("ERROR: No se pudo abrir el archivo de la bitacora");
                    }

                    if (pattern_id == -1) {
                        write_entry(pattern_id, client_message, ipstr);
                    }
                    else {
                        write_entry(pattern_id, client_message, ipstr);
                    }
                    
                    CLEAR(client_message);
                    fclose(binnacle_fd);

                    pthread_mutex_unlock(&mutex);
                }
                else if (read_size == 0) {
                    pthread_mutex_lock(&mutex);
                    binnacle_fd = fopen(binnacle, "a+");
                    write_entry(1, "Communication Offline.", ipstr);
                    fclose(binnacle_fd);
                    pthread_mutex_unlock(&mutex);
                    printf("\nATM de id: %s desconectado.\n", ipstr);
                    fflush(stdout);
                    conectado = 0;
                }
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
