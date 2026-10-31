
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

struct client_info
{
    struct sockaddr_in addr;
};

int main(void)
{
    int serverfd;
    struct sockaddr_in server_addr;
    struct client_info clients[MAX_CLIENTS];
    int client_count = 0;

    char msg[BUFFER_SIZE];
    char command[BUFFER_SIZE];

    serverfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (serverfd == -1)
    {
        perror("socket");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverfd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) == -1)
    {
        perror("bind");
        close(serverfd);
        return 1;
    }

    printf("UDP server listening on port %d\n", PORT);
    printf("Waiting for clients...\n\n");

    while (1)
    {
        fd_set readfds;

        FD_ZERO(&readfds);
        FD_SET(serverfd, &readfds);
        FD_SET(STDIN_FILENO, &readfds);

        int maxfd = serverfd > STDIN_FILENO
                    ? serverfd : STDIN_FILENO;

        int ready = select(maxfd + 1, &readfds,
                           NULL, NULL, NULL);

        if (ready == -1)
        {
            if (errno == EINTR)
                continue;

            perror("select");
            break;
        }

        /*
         * 1. Process incoming UDP messages first.
         */
        if (FD_ISSET(serverfd, &readfds))
        {
            struct sockaddr_in sender;
            socklen_t sender_len = sizeof(sender);

            int n = recvfrom(
                serverfd,
                msg,
                sizeof(msg) - 1,
                0,
                (struct sockaddr *)&sender,
                &sender_len
            );

            if (n == -1)
            {
                perror("recvfrom");
            }
            else
            {
                msg[n] = '\0';

                int id = -1;

                /* Find an already registered client. */
                for (int i = 0; i < client_count; i++)
                {
                    if (clients[i].addr.sin_addr.s_addr ==
                            sender.sin_addr.s_addr &&
                        clients[i].addr.sin_port ==
                            sender.sin_port)
                    {
                        id = i;
                        break;
                    }
                }

                /* Register a new client if necessary. */
                if (id == -1)
                {
                    if (client_count < MAX_CLIENTS)
                    {
                        id = client_count;

                        clients[id].addr = sender;
                        client_count++;

                        printf("\nNew client registered: ID %d\n",
                               id + 1);
                    }
                    else
                    {
                        printf("\nMaximum number of clients reached.\n");
                    }
                }

                if (id != -1)
                {
                    char ip_str[INET_ADDRSTRLEN];

                    if (inet_ntop(AF_INET, &sender.sin_addr,
                                  ip_str, sizeof(ip_str)) == NULL)
                    {
                        perror("inet_ntop");
                        continue;
                    }

                    printf("[Client %d | %s:%u] %s\n",
                           id + 1,
                           ip_str,
                           (unsigned int)ntohs(sender.sin_port),
                           msg);

                    printf("Reply format: ID message\n");
                    printf("Your command: ");
                    fflush(stdout);
                }
            }
        }

        /*
         * 2. Read your manual command when the keyboard is ready.
         */
        if (FD_ISSET(STDIN_FILENO, &readfds))
        {
            printf("\nYour command: ");
            fflush(stdout);

            if (fgets(command, sizeof(command), stdin) == NULL)
            {
                printf("\nStopping server.\n");
                break;
            }

            command[strcspn(command, "\n")] = '\0';

            if (strcmp(command, "quit") == 0)
            {
                printf("Stopping server.\n");
                break;
            }

            char *endptr;
            long id = strtol(command, &endptr, 10);

            if (endptr == command ||
                (*endptr != ' ' && *endptr != '\t'))
            {
                printf("Invalid command. Use: ID message\n");
                continue;
            }

            while (*endptr == ' ' || *endptr == '\t')
                endptr++;

            if (id < 1 || id > client_count)
            {
                printf("Unknown client ID.\n");
                continue;
            }

            if (*endptr == '\0')
            {
                printf("Message cannot be empty.\n");
                continue;
            }

            int target = (int)id - 1;

            int sent = sendto(
                serverfd,
                endptr,
                strlen(endptr),
                0,
                (struct sockaddr *)&clients[target].addr,
                sizeof(clients[target].addr)
            );

            if (sent == -1)
            {
                perror("sendto");
            }
            else
            {
                printf("Reply sent to client %ld.\n", id);
            }
        }
    }

    close(serverfd);
    return 0;
}

