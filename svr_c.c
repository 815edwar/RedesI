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
#include <unistd.h> // for close


#define CLEAR(x) memset(x, '\0', 2000);

int main(int argc, char *argv[]) {
    char *central_module = ""; 
    char message1[2000], server_reply[2000]; 
    struct sockaddr_in server;
    int svr_port = -1;
    int svr_local_port = 20855;
    int error = 0;
    int c, sock;
    char opcion[2000];

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
        printf("\n 13. Introducir una operacion a realizar. ");
        printf("\n 14. Salir. ");
        printf("\n\n Introduzca su opcion: ");
        fgets(opcion, 2000, stdin);
        switch(atoi(opcion)){
        char message1[2000];
        case 1:
            strcpy(message1, "Communication Offline" );
            //message1 = "Communication Offline. \n";
            // Enviando informacion de la opcion 1
            if(send(sock, message1, strlen(message1), 0) < 0){
                puts("Solicitud enviada");
            }
            CLEAR(message1);
            printf("-------------------------\n");
            printf("Enviando mensaje a SVR...\n");
            printf("-------------------------\n");
            break;
        case 2:
            strcpy(message1, "Communication error");
            // Enviando informacion de la opcion 2
            if(send(sock, message1, strlen(message1), 0) < 0){
                puts("Solicitud enviada");
            }
            CLEAR(message1);
            printf("-------------------------\n");
            printf("Enviando mensaje a SVR...\n");
            printf("-------------------------\n");
            break;
        case 3:
            strcpy(message1, "Low Cash alert");
            // Enviando informacion de la opcion 3
            if(send(sock, message1, strlen(message1), 0) < 0){
                puts("Solicitud enviada");
            }
            CLEAR(message1);
            printf("-------------------------\n");
            printf("Enviando mensaje a SVR...\n");
            printf("-------------------------\n");
            break;
        case 4:
            strcpy(message1, "Running Out of notes in cassette");
            // Enviando informacion de la opcion 4
            if(send(sock, message1, strlen(message1), 0) < 0){
                puts("Solicitud enviada");
            }
            CLEAR(message1);
            printf("-------------------------\n");
            printf("Enviando mensaje a SVR...\n");
            printf("-------------------------\n");
            break;
        case 5:
            strcpy(message1,"empty");
            // Enviando informacion de la opcion 5
            if(send(sock, message1, strlen(message1), 0) < 0){
                puts("Solicitud enviada");
            }
            CLEAR(message1);
            printf("-------------------------\n");
            printf("Enviando mensaje a SVR...\n");
            printf("-------------------------\n");
            break;
        case 6: 
            strcpy(message1,"Service mode entered");   
            // Enviando informacion de la opcion 6
            if(send(sock, message1, strlen(message1), 0) < 0){
                puts("Solicitud enviada");
            }
            CLEAR(message1);
            printf("-------------------------\n");
            printf("Enviando mensaje a SVR...\n");
            printf("-------------------------\n");
            break;
        case 7:
            strcpy(message1,"Service mode left");
            // Enviando informacion de la opcion 7
            if(send(sock, message1, strlen(message1), 0) < 0){
                puts("Solicitud enviada");
            }
            CLEAR(message1);
            printf("-------------------------\n");
            printf("Enviando mensaje a SVR...\n");
            printf("-------------------------\n");
            break;
        case 8:
            strcpy(message1,"device did not answer as expected");
            // Enviando informacion de la opcion 8
            if(send(sock, message1, strlen(message1), 0) < 0){
                puts("Solicitud enviada");
            }
            CLEAR(message1);
            printf("-------------------------\n");
            printf("Enviando mensaje a SVR...\n");
            printf("-------------------------\n");
            break;
        case 9:
            strcpy(message1,"The protocol was cancelled");
            // Enviando informacion de la opcion 9
            if(send(sock, message1, strlen(message1), 0) < 0){
                puts("Solicitud enviada");
            }
            CLEAR(message1);
            printf("-------------------------\n");
            printf("Enviando mensaje a SVR...\n");
            printf("-------------------------\n");
            break;
        case 10:
            strcpy(message1,"Low Paper warning");
            // Enviando informacion de la opcion 10
            if(send(sock, message1, strlen(message1), 0) < 0){
                puts("Solicitud enviada");
            }
            CLEAR(message1);
            printf("-------------------------\n");
            printf("Enviando mensaje a SVR...\n");
            printf("-------------------------\n");
            break;
        case 11:
            strcpy(message1,"Printer Error");
            // Enviando informacion de la opcion 11
            if(send(sock, message1, strlen(message1), 0) < 0){
                puts("Solicitud enviada");
            }
            CLEAR(message1);
            printf("-------------------------\n");
            printf("Enviando mensaje a SVR...\n");
            printf("-------------------------\n");
            break;
        case 12:
            strcpy(message1,"Paper-out condition");
            // Enviando informacion de la opcion 12
            if(send(sock, message1, strlen(message1), 0) < 0){
                puts("Solicitud enviada");
            }
            CLEAR(message1);
            printf("-------------------------\n");
            printf("Enviando mensaje a SVR...\n");
            printf("-------------------------\n");
            break;
        case 13:
            printf("-------------------------\n");
            printf(" Introduzca la operacion: ");
            fgets(message1, 2000, stdin);
            // Enviando informacion de la opcion 13
            if (send(sock, message1, strlen(message1), 0) < 0){
                puts("Solicitud Fallida");
                return 1;
            }
            CLEAR(message1);
            printf("-------------------------\n");
            printf("Enviando mensaje a SVR...\n");
            printf("-------------------------\n");
            break;
        case 14:
            printf("-------------------------\n");
            printf("Hasta luego vuelva pronto\n");
            close(sock);
            return 0;
        default:
            printf("-------------------------\n");
            printf("La opcion introducida es invalida. Vuelva a intentarlo\n");
            break;
        }

        CLEAR(opcion);
    }
    while(1);

}