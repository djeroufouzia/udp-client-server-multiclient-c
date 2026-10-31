
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024

int sockfd;

void *receive_messages(void *arg)
{
    char buffer[BUFFER_SIZE];
    struct sockaddr_in server_addr;
    socklen_t server_len = sizeof(server_addr);

    while (1)
    {
        int n = recvfrom(
            sockfd,
            buffer,
            sizeof(buffer) - 1,
            0,
            (struct sockaddr *)&server_addr,
            &server_len
        );

        if (n == -1)
        {
            perror("recvfrom");
            break;
        }

        buffer[n] = '\0';
        printf("\n[Server]: %s\n", buffer);
        printf("Your message: ");
        fflush(stdout);
    }

    return NULL;
}

int main(void)
{
    struct sockaddr_in server_addr;
    pthread_t receiver_thread;
    char message[BUFFER_SIZE];

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd == -1)
    {
        perror("socket");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP,
                  &server_addr.sin_addr) != 1)
    {
        fprintf(stderr, "Invalid server IP address.\n");
        close(sockfd);
        return 1;
    }

    if (pthread_create(&receiver_thread, NULL,
                       receive_messages, NULL) != 0)
    {
        perror("pthread_create");
        close(sockfd);
        return 1;
    }

    printf("Connected to UDP server %s:%d\n",
           SERVER_IP, PORT);
    printf("Type messages and press Enter. Type 'quit' to exit.\n");

    while (1)
    {
        printf("Your message: ");
        fflush(stdout);

        if (fgets(message, sizeof(message), stdin) == NULL)
            break;

        message[strcspn(message, "\n")] = '\0';

        if (strcmp(message, "quit") == 0)
            break;

        int sent = sendto(
            sockfd,
            message,
            strlen(message),
            0,
            (struct sockaddr *)&server_addr,
            sizeof(server_addr)
        );

        if (sent == -1)
        {
            perror("sendto");
            break;
        }
    }

    close(sockfd);
    pthread_cancel(receiver_thread);
    pthread_join(receiver_thread, NULL);

    printf("Client stopped.\n");
    return 0;
}

