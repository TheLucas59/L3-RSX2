#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PORT 53
#define DNS_ADDR "193.49.225.15"
#define BUFFER_SIZE 1024
#define REQUEST_LENGTH 29

void sendDNSRequest(int socket_dns, struct sockaddr_in dest) {
    const char request[REQUEST_LENGTH] = {
        0x08, 0xbb, 0x01, 0x00,    //entête
        0x00, 0x01, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x03, 0x77, 0x77, 0x77, //question    
        0x04, 0x6c, 0x69, 0x66, //QNAME : 3"www" 4"lifl" 2"fr" 0 
        0x6c, 0x02, 0x66, 0x72,
        0x00,
        0x00, 0x01,                //Q-Type
        0x00, 0x01                 //Q-Class
    };
    if( sendto(socket_dns, request, REQUEST_LENGTH, 0, (struct sockaddr*) &dest, sizeof(dest)) == -1) {
        perror("sendto");
        exit(EXIT_FAILURE);
    }
}

int main() {
    int socket_dns = socket(AF_INET, SOCK_DGRAM, 0);
    if(socket_dns == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in dest;
    dest.sin_family = AF_INET;
    dest.sin_port = htons(PORT);

    in_addr_t adresseDest = inet_addr(DNS_ADDR);
    if(adresseDest == INADDR_NONE) {
        printf("Adresse IP invalide.\n");
        exit(EXIT_FAILURE);
    }

    dest.sin_addr.s_addr = adresseDest;

    sendDNSRequest(socket_dns, dest);
    printf("Message envoyé.\n");

    struct sockaddr_in addrRecv;
    socklen_t addrRecvlen = sizeof(struct sockaddr_in);

    char buffer[BUFFER_SIZE];
    int received = 0;

    if ((received = recvfrom(socket_dns, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &addrRecv, &addrRecvlen)) == -1) {
      perror("recvfrom");
      exit(EXIT_FAILURE);
    }

    printf(" - port  distant  : %hu\n", ntohs(addrRecv.sin_port));
    printf(" - @IPv4 distante : %s\n", inet_ntoa(addrRecv.sin_addr));
    printf("longueur du message recu : %u\n", received);

    //printf("%s\n", buffer);

    printf("Message reçu.\n");
    close(socket_dns);

    for (int i = 0; i < received; i++) {

        fprintf(stdout," %.2X", buffer[i] & 0xff);

        if (((i+1)%16 == 0) || (i+1 == received)) {

            /* ceci pour afficher les caracteres ascii apres l'hexa */
            /* >>> */
            for (int j = i+1 ; j < ((i+16) & ~15); j++) {
            fprintf(stdout,"   ");
            }
            fprintf(stdout,"\t");
            for (int j = i & ~15; j <= i; j++)
            fprintf(stdout,"%c",buffer[j] > 31 && buffer[j] < 128 ? (char)buffer[j] : '.');
            /* <<< */
            fprintf(stdout,"\n");
        }
    }


    return 0;
}