#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    if(argc != 4) {
        printf("Nombre d'arguments incorrect.\n");
        exit(1);
    }

    int socket_send = socket(AF_INET, SOCK_DGRAM, 0);
    if(socket_send == -1) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in dest;
    dest.sin_family = AF_INET;
    dest.sin_port = htons(atoi(argv[2]));

    in_addr_t adresseDest = inet_addr(argv[1]);
    if(adresseDest == INADDR_NONE) {
        printf("Adresse IP invalide.\n");
        exit(1);
    }

    dest.sin_addr.s_addr = adresseDest;

    if(sendto(socket_send, argv[3], strlen(argv[3]), 0, (struct sockaddr*) &dest, sizeof(dest)) == -1) {
        perror("sendto");
        exit(1);
    }

    printf("Message envoyé.\n");
    close(socket_send);

    return 0;
}