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

//the thread function
void *connection_handler(void *);

#define CLEAR(x) memset(x, '\0', 2000);
FILE *binnacle_fd;
char *binnacle = "";

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
    server.sin_addr.s_addr = inet_addr("192.168.2.20");
    server.sin_port = htons( svr_port );
    
    /* Se enlaza el socket a la direccion de entrada 
     * en caso de qie no se logre enlazar, se aborta la ejecucion del programa */
    if (bind(socket_desc,(struct sockaddr *) &server , sizeof(server)) < 0) {
        perror("El enlace del socket con la direccion de entrada fallo. ");
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
        */ 
        if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void *) new_sock) < 0) {
            perror("could not create thread");
            return 1;
        }
        
        /* Espera que los hilos creados terminen su ejecucion para terminar el */ 
        pthread_join( sniffer_thread , NULL);
        puts("Handler assigned");
    }
     
    if (client_sock < 0) {
        perror("accept failed");
        return 1;
    }

    fclose(binnacle_fd);
     
    return 0;
}

int catch_pattern(char *message) {
    regex_t regex;
    int reti;
    char msgbuf[100];

    reti = regcomp(&regex, ".*domingo", 0);
    if (reti) {
        fprintf(stderr, "Could not compile regex\n");
        exit(1);
    }

    /* Execute regular expression */
    reti = regexec(&regex, "edwar y domingo y mafer", 0, NULL, 0);
    if (!reti) {
        puts("Match");
    }
    else if (reti == REG_NOMATCH) {
        puts("No match");
    }
    else {
        regerror(reti, &regex, msgbuf, sizeof(msgbuf));
        fprintf(stderr, "Regex match failed: %s\n", msgbuf);
        exit(1);
    }

    /* Free memory allocated to the pattern buffer by regcomp() */
    regfree(&regex);
}

/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc) {
    //Get the socket descriptor
    int sock = *(int *) socket_desc;
    int read_size, port;
    char *message , client_message[2000];
    struct sockaddr_storage addr;
    char ipstr[INET6_ADDRSTRLEN];
    socklen_t len;

    len = sizeof addr;
    getpeername(sock, (struct sockaddr *) &addr, &len);

    struct sockaddr_in *s = (struct sockaddr_in *) &addr;
    port = ntohs(s->sin_port);
    inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);

    printf("%s\n", ipstr);
     
    //Receive a message from client
    while( (read_size = recv(sock, client_message, 2000, 0)) > 0 ) {
        //Send the message back to client
        // write(sock , client_message , strlen(client_message));
        if ((binnacle_fd = fopen(binnacle, "a+")) < 0) {
            perror("ERROR: No se pudo abrir el archivo de la bitacora");
        }

        puts(client_message);
        fputs(client_message, binnacle_fd);
        fclose(binnacle_fd);
        CLEAR(client_message);
    }
     
    if (read_size == 0) {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if (read_size == -1) {
        perror("recv failed");
    }
         
    //Free the socket pointer   
    free(socket_desc);
     
    return 0;
}
