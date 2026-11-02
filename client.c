
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define SERVER_IP "192.168.56.7"
#define PORT 8080
#define BUFFER_SIZE 1024

int sockfd;

struct sockaddr_in server_addr;

/* Thread responsible for receiving messages from the server */
void *receive_messages(void *arg)
{
    (void)arg;

    char buffer[BUFFER_SIZE];

    while (1)
    {
        struct sockaddr_in sender_addr;
        socklen_t sender_len = sizeof(sender_addr);

        fprintf(stderr, "\n Waiting for server response\n");
        fflush(stderr);

        ssize_t n = recvfrom(
            sockfd,
            buffer,
            sizeof(buffer) - 1,
            0,
            (struct sockaddr *)&sender_addr,
            &sender_len
        );

        if (n < 0)
        {
            perror("[ERROR] recvfrom");
            break;
        }

        buffer[n] = '\0';

        char sender_ip[INET_ADDRSTRLEN];

        inet_ntop(
            AF_INET,
            &sender_addr.sin_addr,
            sender_ip,
            sizeof(sender_ip)
        );

        printf("[Server %s:%d]: %s\n",
               sender_ip,
               ntohs(sender_addr.sin_port),
               buffer);

        printf("Your message: ");
        fflush(stdout);
    }

    return NULL;
}

int main(void)
{
    pthread_t receiver_thread;
    char message[BUFFER_SIZE];

    /* Create UDP socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0)
    {
        perror("socket");
        return EXIT_FAILURE;
    }

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP,
                  &server_addr.sin_addr) != 1)
    {
        fprintf(stderr,"Invalid server IP address.\n");
        close(sockfd);
        return EXIT_FAILURE;
    }

    /* Start receiver thread */
    if (pthread_create(&receiver_thread, NULL,
                       receive_messages, NULL) != 0)
    {
        fprintf(stderr,"Error creating receiver thread.\n");
        close(sockfd);
        return EXIT_FAILURE;
    }

    
    printf("Server: %s:%d\n", SERVER_IP, PORT);


    printf("Type messages. Type 'quit' to exit.\n\n");

    while (1)
    {
        printf("Your message: ");
        fflush(stdout);

        if (fgets(message, sizeof(message), stdin) == NULL)
            break;

        message[strcspn(message, "\n")] = '\0';

        if (strcmp(message, "quit") == 0)
            break;

        if (strlen(message) == 0)
            continue;

        /* Send message to UDP server */
        ssize_t sent = sendto(
            sockfd,
            message,
            strlen(message),
            0,
            (struct sockaddr *)&server_addr,
            sizeof(server_addr)
        );

        if (sent < 0)
        {
            perror("[ERROR] sendto");
            break;
        }

        printf("Sent %zd bytes to %s:%d: %s\n",
               sent, SERVER_IP, PORT, message);

        fflush(stdout);
    }

    /* Stop receiver thread */
    pthread_cancel(receiver_thread);
    pthread_join(receiver_thread, NULL);

    close(sockfd);

    printf("\nClient stopped.\n");

    return 0;
}

