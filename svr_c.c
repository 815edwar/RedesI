/*************************************************************************
 *                            Cliente ATM                                *
 * Authors:                                                              *
 * Edwar Yepez              12-10855                                     *
 * Maria Fernanda Machado   13-10780                                     *
 * Veronica Mazutiel        13-10853                                     *
 *                                                                       *
 *************************************************************************/

// Libraries imports
#include <stdio.h>          //printf
#include <string.h>         //strlen
#include <sys/socket.h>     //socket
#include <arpa/inet.h>      //inet_addr
#include <getopt.h>         //get arguments
#include <stdlib.h>         //atoi
#include <string.h>         //strcmp
#include <math.h>

int main(int argc, char *argv[]) {
    char *central_module = ""; 
    char message[1000], server_reply[2000]; 
    struct sockaddr_in server;
    int svr_port = -1;
    int svr_local_port = 20855;
    int error = 0;
    int c, sock;

    int opcion;

    while( ( c = getopt( argc, argv, "d: p: l:" ) ) != -1 ) {
        switch(c) {
            case 'd':
                central_module = optarg;
                break;
            case 'p':
                svr_port = atoi(optarg);
                break;
            case 'l':
                svr_local_port = atoi(optarg);
                break;
            case '?':
                error++;
        }
    }

    if (svr_port == -1 || strcmp(central_module, "") == 0) error++;

    if (error) {
        perror("\n\tERROR: el formato valido es: "
               "./svr_c -d <nombre_módulo_central> -p <puerto_svr_s> [-l <puerto_local>]"
               "\n\tTodos los parametros entre [] son opcionales.\n");
        exit(0);
    }

    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1) {
        printf("Could not create socket");
    }

    puts("Socket created");
     
    server.sin_addr.s_addr = inet_addr(central_module);
    server.sin_family = AF_INET;
    server.sin_port = htons(svr_port);
    
    //Connect to remote server
    if (connect(sock , (struct sockaddr *) &server , sizeof(server)) < 0) {
        perror("connect failed. Error");
        return 1;
    }
     
    puts("Connected\n");
     
    //keep communicating with server
    do{
        printf("\n 1. Communication Offline.");
        printf("\n 2. Communication Error.");
        printf("\n 3. Low Cash alert.");
        printf("\n 4. Running Out of notes in cassette.");
        printf("\n 5. Empty.");
        printf("\n 6. Service mode entered. ");
        printf("\n 7. Service mode left. ");
        printf("\n 8. Device did not answer as expected. ");
        printf("\n 9. The protocol was cancelled. ");
        printf("\n 10. Low paper warning. ");
        printf("\n 11. Printer Error. ");
        printf("\n 12. Paper-Out condition. ");
        printf("\n\n Introduzca su opcion: ");
        scanf("%d", &opcion);
        switch(opcion){
        case 1:
            //fgets(message, 1000,stdin);
            // Enviando informacion de la opcion1
            if(send(sock, opcion, strlen(opcion), 0) < 0){
            puts("Solicitud enviada");
            return 1;
            }
        case 2:
            //fgets(opcion, 1000,stdin);
            // Enviando informacion de la opcion2
            if(send(sock, opcion, strlen(opcion), 0) < 0){
            puts("Solicitud enviada");
            return 1;
            }
        case 3:
            //fgets(opcion, 1000,stdin);
            // Enviando informacion de la opcion3
            if(send(sock, opcion, strlen(opcion), 0) < 0){
            puts("Solicitud enviada");
            return 1;
            }
        case 4:
            //fgets(opcion, 1000,stdin);
            // Enviando informacion de la opcion1
            if(send(sock, opcion, strlen(opcion), 0) < 0){
            puts("Solicitud enviada");
            return 1;
            }
        case 5:
            //fgets(opcion, 1000,stdin);
            // Enviando informacion de la opcion1
            if(send(sock, opcion, strlen(opcion), 0) < 0){
            puts("Solicitud enviada");
            return 1;
            }
        case 6:    
            //fgets(opcion, 1000,stdin);
            // Enviando informacion de la opcion1
            if(send(sock, opcion, strlen(opcion), 0) < 0){
            puts("Solicitud enviada");
            return 1;
            }
        case 7:
            //fgets(opcion, 1000,stdin);
            // Enviando informacion de la opcion1
            if(send(sock, opcion, strlen(opcion), 0) < 0){
            puts("Solicitud enviada");
            return 1;
            }
        case 8:
            //fgets(opcion, 1000,stdin);
            // Enviando informacion de la opcion1
            if(send(sock, opcion, strlen(opcion), 0) < 0){
            puts("Solicitud enviada");
            return 1;
            }
        case 9:
            fgets(opcion, 1000,stdin);
            // Enviando informacion de la opcion1
            if(send(sock, opcion, strlen(opcion), 0) < 0){
            puts("Solicitud enviada");
            return 1;
            }
        case 10:
            fgets(opcion, 1000,stdin);
            // Enviando informacion de la opcion1
            if(send(sock, opcion, strlen(opcion), 0) < 0){
            puts("Solicitud enviada");
            return 1;
            }
        case 11:
            fgets(opcion, 1000,stdin);
            // Enviando informacion de la opcion1
            if(send(sock, opcion, strlen(opcion), 0) < 0){
            puts("Solicitud enviada");
            return 1;
            }
        case 12:
            //fgets(opcion, 1000,stdin);
            // Enviando informacion de la opcion1
            if(send(sock, opcion, strlen(opcion), 0) < 0){
            puts("Solicitud enviada");
            return 1;
            }
        }
    }
    while(opcion != 13);
    //close(sock);
    //return 0;

    
    /*while(1) {
        printf("Enter message to send:\n");
        fgets(message, 1000, stdin);

        //Send some data
        if (send(sock, message, strlen(message), 0) < 0) {
            puts("Send failed");
            return 1;
        }
    }
     
    close(sock);
    return 0;*/

}