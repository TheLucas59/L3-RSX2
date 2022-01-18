#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define LOCALHOST "127.0.0.1"
#define BUFF_SIZE 256

int main(int argc, char* argv[]) {
    if(argc != 2) {
        printf("Nombre d'arguments incorrect.\n");
        exit(1);
    }

    int socket_receive = socket(AF_INET, SOCK_DGRAM, 0);
    if(socket_receive == -1) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in here;
    here.sin_family = AF_INET;
    here.sin_port = htons(atoi(argv[1]));

    in_addr_t adresseHere = inet_addr(LOCALHOST);
    if(adresseHere == INADDR_NONE) {
        printf("Adresse IP invalide.\n");
        exit(1);
    }

    here.sin_addr.s_addr = adresseHere;

    if(bind(socket_receive, (struct sockaddr*) &here, sizeof(here)) == -1) {
        perror("bind");
        exit(1);
    }

    char msg_buff[BUFF_SIZE];
    if(recv(socket_receive, msg_buff, BUFF_SIZE, 0) == -1) {
        perror("recv");
        exit(1);
    }

    printf("Message reçu : %s\n", msg_buff);
    close(socket_receive);

    return 0;
}