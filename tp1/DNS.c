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
#define QDCOUNT_INDEX 4
#define ANCOUNT_INDEX 6
#define DNS_HEADER_LENGTH 12
#define DNS_END_LENGTH 4
#define IP_ADDRESS_LENGTH 4
#define TYPE_A 1
#define TYPE_CNAME 5
#define CLASS_IN 1
#define SEPARATOR '.'

int bitExtracted(int number, int n, int pos) {
    return (((1 << n) - 1) & (number >> (pos - 1)));
}

unsigned char * concat(unsigned char *dest, const char *src, int n, int length) {
    int i = 0;

    for (; i < n; i++) {
        dest[length + i] = src[i];
    }

    return dest;
}

int sendDNSRequest(int socket_dns, struct sockaddr_in dest, char* name) {
    const char header[DNS_HEADER_LENGTH] = {
        0x08, 0xbb, 0x01, 0x00,    //entête
        0x00, 0x01, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    };

    const char end[DNS_END_LENGTH] = {
        0x00, 0x01,                //Q-Type
        0x00, 0x01                 //Q-Class
    };

    int name_full_length = strlen(name) + 1;
    char qname[name_full_length];

    int pos = 0;
    char cptBeforeSeparator = 0x00;
    for(int i = 0; i < name_full_length; i++) {
        if(name[i] != SEPARATOR && name[i] != '\0') {
            cptBeforeSeparator++;
        }
        else {
            qname[pos] = cptBeforeSeparator;
            int nextEnd = pos + cptBeforeSeparator;
            pos++;
            for(; pos <= nextEnd; pos++) {
                qname[pos] = name[pos-1];
            }
            cptBeforeSeparator = 0x00;
        }
    }
    qname[pos] = 0x00;


    int request_size = DNS_HEADER_LENGTH + name_full_length + DNS_END_LENGTH + 1;
    unsigned char* req = malloc(sizeof(char)* request_size);

    concat(req, header, DNS_HEADER_LENGTH, 0);
    concat(req, qname, name_full_length+1, DNS_HEADER_LENGTH);
    concat(req, end, DNS_END_LENGTH, DNS_HEADER_LENGTH + name_full_length+1);
    req[request_size] = 0x00;

    int length_sent = 0;
    if( (length_sent = sendto(socket_dns, req, request_size, 0, (struct sockaddr*) &dest, sizeof(dest))) == -1) {
        perror("sendto");
        exit(EXIT_FAILURE);
    }

    return length_sent;
}

void analyseDNSResponse(unsigned char* buffer, int length_sent) {
    int ANCount = (buffer[ANCOUNT_INDEX] << 8) + buffer[ANCOUNT_INDEX+1];
    
    int pos = length_sent;
    int cpt = 0;

    //Answer section
    while(cpt < ANCount) {
        int val = buffer[pos];
        //NAME : variable length
        if( (val & 1<<7) && (val & 1<<6) ) {
            //pointer on 2 bytes
            pos +=2;
        }
        else {
            for(; pos < BUFFER_SIZE; pos++) {
                if(buffer[pos] == 0x00) {
                    break;
                }
                else {
                    pos += buffer[pos];
                }
            }
        }
        //TYPE : 2 bytes
        int type = (buffer[pos] << 8) + buffer[pos+1];
        pos += 2;


        //CLASS : 2 bytes
        int class = (buffer[pos] << 8) + buffer[pos+1];
        pos += 2;

        //TTL : 4 bytes
        pos += 4;

        //RDLENGTH : 2 bytes
        int length = (buffer[pos] << 8) + buffer[pos+1];
        pos += 2;


        //RDATA if it contains ip address
        if(length == 4 && type == TYPE_A && class == CLASS_IN) {
            int addressBytes[IP_ADDRESS_LENGTH];
            for(int i = 0; i < IP_ADDRESS_LENGTH; i++) {
                addressBytes[i] = buffer[pos];
                pos++;
            }
            printf("Adresse possible : %d.%d.%d.%d\n", addressBytes[0], addressBytes[1], addressBytes[2], addressBytes[3]);
        }
        else if(type == TYPE_CNAME && class == CLASS_IN) {
            char cnameBytes[BUFFER_SIZE];
            int nbCharacterBeforeNextSeparator = 0;
            int cnamePos = 0;
            short cnameTerminatedByPointer = 0;
            while(buffer[pos] != 0x00 && !cnameTerminatedByPointer) {
                if( (buffer[pos] & 1<<7) && (buffer[pos] & 1<<6) ) {
                    cnameTerminatedByPointer = 1;
                    int name_field = (buffer[pos] << 8) + buffer[pos+1];
                    int offset = bitExtracted(name_field, 14, 1);
                    while(buffer[offset] != 0x00) {
                        nbCharacterBeforeNextSeparator = buffer[offset];
                        offset++;
                        for(int i = 0; i < nbCharacterBeforeNextSeparator; i++) {
                            cnameBytes[cnamePos] = buffer[offset];
                            offset++;
                            cnamePos++;
                        }
                        cnameBytes[cnamePos] = '.';
                        cnamePos++;
                    }
                    pos += 2;
                }
                else {
                    nbCharacterBeforeNextSeparator = buffer[pos];
                    pos++;
                    for(int i = 0; i < nbCharacterBeforeNextSeparator; i++) {
                        cnameBytes[cnamePos] = buffer[pos];
                        pos++;
                        cnamePos++;
                    }
                    cnameBytes[cnamePos] = '.';
                    cnamePos++;
                }
            }
            printf("Alias pour : %s\n", cnameBytes);
        }
        else {
            pos += length;
        }
        cpt++;
    }
}

int main(int argc, char* argv[]) {
    if(argc < 2 || argc > 3) {
        printf("Mauvais nombre d'arguments : utilisez le programme ainsi ./DNS <adresse_recherchée> <adresse_serveur_DNS>.\n");
        exit(EXIT_FAILURE);
    }

    int socket_dns = socket(AF_INET, SOCK_DGRAM, 0);
    if(socket_dns == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in dest;
    dest.sin_family = AF_INET;
    dest.sin_port = htons(PORT);

    in_addr_t adresseDest;
    if(argc == 3) {
        adresseDest = inet_addr(argv[2]);
        if(adresseDest == INADDR_NONE) {
            adresseDest = inet_addr(DNS_ADDR);
            if(adresseDest == INADDR_NONE) {
                printf("Adresse IP invalide.\n");
                exit(EXIT_FAILURE);
            }
        }

    }
    else {
        adresseDest = inet_addr(DNS_ADDR);
        if(adresseDest == INADDR_NONE) {
            printf("Adresse IP invalide.\n");
            exit(EXIT_FAILURE);
        }
    }

    dest.sin_addr.s_addr = adresseDest;

    int length_sent = sendDNSRequest(socket_dns, dest, argv[1]);
    printf("Message envoyé.\n");

    struct sockaddr_in addrRecv;
    socklen_t addrRecvlen = sizeof(struct sockaddr_in);

    unsigned char buffer[BUFFER_SIZE];
    int received = 0;

    if ((received = recvfrom(socket_dns, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &addrRecv, &addrRecvlen)) == -1) {
      perror("recvfrom");
      exit(EXIT_FAILURE);
    }

    printf("Message reçu.\n");
    close(socket_dns);

    printf("Affichage de la trame en hexadecimal : \n");
    int j = 0;
    for (int i = 0; i < received; i++) {
        printf("%.2X ", buffer[i]);
        j++;
        if(j == 15) {
            printf("\n");
            j = 0;
        }
    }
    printf("\n");

    analyseDNSResponse(buffer, length_sent);

    return 0;
}