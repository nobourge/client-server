/* Chat room program
 * Auteurs : Noe Bourgeois, Morari Augustin-Constantin
 * Date : 19/12/2021
 */

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/select.h>
#include <stdlib.h>  // for strtol
#include <stdbool.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>  //close

#include "common.h"

#define PSEUDO_LEN_MAX 25
#define CLIENTS_QUANTITY_MAX 1024

/// Serveur initialise les arguments, le socket et recois et envoie des messages
/// \param argc
/// \param argv
/// \return 0
int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("You have entered %d  arguments: \n", argc);
        printf("USAGE: $ ./server <port> \n %d", argc);
        exit(1);
    }

    const char *temp_port = argv[1];
    // converting char to int
    int port = conv_port(temp_port);

    int opt = 1;
    int master_socket = checked(socket(AF_INET, SOCK_STREAM, 0));
    checked(setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)));

    // type of socket created
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    checked(bind(master_socket, (struct sockaddr *)&address, sizeof(address)));
    checked(listen(master_socket, 3));

    size_t addrlen = sizeof(address);
    fd_set readfds;
    fd_set writefds;

    int clients[BUFF_SIZE];
    int nclients = 0;

    char **pseudos;

    // allocation de memoire pour la database
    pseudos = malloc(CLIENTS_QUANTITY_MAX * sizeof(char*));
    for (int i = 0; i < CLIENTS_QUANTITY_MAX; i++){
        pseudos[i] = malloc((PSEUDO_LEN_MAX+1) * sizeof(char));
        strcpy(pseudos[i], "NULL");
    }

    while (true)
    {
        // add new sockets to fd_set
        FD_ZERO(&readfds);
        FD_SET(master_socket, &readfds);
        int max_fd = master_socket;
        for (int i = 0; i < nclients; i++)
        {
            FD_SET(clients[i], &readfds);
            FD_SET(clients[i], &writefds);
            if (clients[i] > max_fd)
            {
                max_fd = clients[i];
            }
        }
        // wait for an activity on one of the sockets,
        // timeout is NULL
        // select avec
        // un fd_set in lecture,
        // un fd_set en ecriture
        select(max_fd + 1, &readfds, &writefds, NULL, NULL);

        if (FD_ISSET(master_socket, &readfds)) {
            // Si c'est le master socket qui a des donnees, c'est une nouvelle connexion.
            clients[nclients] = accept(master_socket, (struct sockaddr *)&address, (socklen_t *)&addrlen);
            nclients++;
        }
        else {
            // Sinon, c'est un message d'un client
            for (int i = 0; i < nclients; i++) {
                if (FD_ISSET(clients[i], &readfds)) {
                    char *buffer;
                    char *timestamp;
                    size_t nbytes = receive(clients[i], (void *)&buffer);
                    size_t timestamp_nbytes = receive(clients[i], (void *)&timestamp);
                    if (nbytes > 0) {
                        // si c'est une premiere connexion...
                        if (strcmp(pseudos[i], "NULL") == 0) {
                            char *pseudo_ptr = buffer;
                            strcpy(pseudos[i], buffer);
                            // pseudo is registered
                            printf("%s User %s connected \n", timestamp, pseudo_ptr);
                        }
                        // si un utilisateur enregistre dans la databe veut envoyer un message...
                        else {
                            printf("%s User %s a dit : %s \n", timestamp, pseudos[i], buffer);
                            size_t len_pseudo = strlen(pseudos[i]);
                            for (int j = 0; j < nclients; j++) {
                                if (FD_ISSET(clients[j], &writefds)) {
                                    ssend(clients[j], pseudos[i], len_pseudo);
                                    ssend(clients[j], buffer, nbytes);
                                    ssend(clients[j], timestamp, timestamp_nbytes);
                                }
                            }
                        }
                        free(buffer);
                        free(timestamp);
                    }
                    else {
                        close(clients[i]);
                        // On deplace le dernier socket a la place de libre pour ne pas faire de trou.
                        clients[i] = clients[nclients - 1];
                        printf("User %s disconnected\n", pseudos[i]);
                        strcpy(pseudos[i], "NULL");
                        nclients--;
                    }
                }
            }
        }
    }
    return 0;
}