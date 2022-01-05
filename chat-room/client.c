/* Chat room program
 * Auteurs : Noe Bourgeois, Morari Augustin-Constantin
 * Date : 19/12/2021
 */

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/select.h>
#include <stdlib.h>  // for strtol
#include <signal.h>
#include <stdbool.h>
#include <pthread.h>

#include "common.h"

#define PROMPT "Entrez votre message: "
#define PROMPT_LEN (strlen(PROMPT))

// structure contenant les mutexes et le socket
typedef struct {
    pthread_mutex_t* mutex;
    int* sock;
} thread_args_t;

/// Recupere les donnees envoyées par le server
/// \param ptr
/// \return
void* freceive(void* ptr) {
    // on recupere les donnees de la structure passee en param
    thread_args_t* args = (thread_args_t*)ptr;
    int* sock = args->sock;

    ssize_t nbytes = 1;

    while (nbytes != 0) {
        char *recvbuffer;
        char *recvtimestamp;
        char *pseudo;
        ssize_t pseudo_nbytes = receive(*sock, (void *) &pseudo);
        nbytes = receive(*sock, (void *) &recvbuffer);
        ssize_t timestamp_nbytes = receive(*sock, (void *) &recvtimestamp);
        if (nbytes > 0 && timestamp_nbytes > 0 && pseudo_nbytes > 0) {
            // clear the line that the cursor is currently on
            printf("\33[2K\r");
            printf("%s User %s sent : %s \n%s\n", recvtimestamp, pseudo, recvbuffer, PROMPT);
            free(recvtimestamp);
            free(recvbuffer);
        }
        // Si la connexion avec le serveur est perdue -> on affiche ce message
        else {
            // clear the line that the cursor is currently on
            printf("\33[2K\r");
            printf("Connexion with the server lost !\nStart the server again !\n");
            exit(0);
        }
    }
    return NULL;
}

/// Envoie les données au server
/// \param ptr pointeur vers le string ecrit en stdin par user
/// \return
void* fssend(void* ptr) {
    // on recupere les donnees de la structure passee en parametre
    thread_args_t* args = (thread_args_t*)ptr;
    int* sock = args->sock;
    pthread_mutex_t* mutex = args->mutex;

    char buffer[BUFF_SIZE];
    char timestamp[TIMESTAMP_SIZE];

    ssize_t nbytes = 1;
    ssize_t timestamp_nbytes = 1;

    printf("%s",PROMPT);
    char *line = fgets(buffer, BUFF_SIZE, stdin);
    while (nbytes != 0 && timestamp_nbytes > 0 && line != NULL) {
        // on initialize le temps
        time_t now = time(NULL);
        strftime(timestamp, 20, "%Y-%m-%d %H:%M:%S", localtime(&now));

        //Longueur du message size_t
        size_t len = strlen(buffer);

        // Supprimer le \n
        buffer[len - 1] = '\0';

        // On garde la même taille de string pour explicitement envoyer le '\0'
        // on bloque le socket quand on veut ecrire qq chose dessus
        pthread_mutex_lock(mutex);
        nbytes = ssend(*sock, buffer, len);
        timestamp_nbytes = ssend(*sock, timestamp, strlen(timestamp));
        // we unlock the socket
        pthread_mutex_unlock(mutex);

        //get new line
        line = fgets(buffer, BUFF_SIZE, stdin);
    }

    if (line == NULL){
        // Si Ctrl+D ->  connexion ended
        // clear the line that the cursor is currently on
        printf("\33[2K\r");
        printf("Client ended session\n");
        exit(0);
    }
    return NULL;
}

/// Initialise les parametres, le socket, les mutexes et les threads
/// \param argc
/// \param argv
/// \return 0
int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("You have entered %d  arguments: \n", argc);
        printf("USAGE: $ ./client <pseudo> <ip_serveur> <port> \n");
        exit(1);
    }
    char *pseudo = argv[1];
    size_t pseudo_len = strlen(pseudo);
    if (pseudo_len < 3 || 25 < pseudo_len ) {
        printf(" <pseudo>  must have between 3 and 25 characters\n");
        exit(1);
    }
    const char *ip = argv[2]; // initialement 127.0.0.1
    const char *temp_port = argv[3]; // initalement 8080
    // converting char to int
    int port = conv_port(temp_port);

    int sock = checked(socket(AF_INET, SOCK_STREAM, 0));
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Conversion de string vers IPv4 ou IPv6 en binaire
    checked(inet_pton(AF_INET, ip, &serv_addr.sin_addr));

    //connection demand
    checked(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)));

    //send pseudo
    char timestamp[TIMESTAMP_SIZE];
    time_t now = time(NULL);
    strftime(timestamp, TIMESTAMP_SIZE, "%Y-%m-%d %H:%M:%S", localtime(&now));
    ssend(sock, pseudo, pseudo_len);
    ssend(sock, timestamp, strlen(timestamp));

    // on initialise le mutex
    pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
    thread_args_t args;
    args.sock = &sock;
    args.mutex = &m;

    // thread ids
    pthread_t tids[2];

    pthread_create(&tids[0], NULL, fssend, &args);
    pthread_create(&tids[1], NULL, freceive, &args);
    pthread_join(tids[0], NULL);
    pthread_join(tids[1], NULL);

    // on detruit le mutex a la fin de l'execution
    pthread_mutex_destroy(&m);

    printf("Program is shutting down.\n");
    return 0;
}