#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define FALSE 0
#define TRUE 1

#define ZeroMemory(Destination, Length) memset(Destination, NULL, Length)

typedef unsigned char BYTE;
typedef unsigned int BOOL;
typedef unsigned long SOCKET;

void RecvThread(SOCKET *sock)
{
    while (TRUE)
    {
        BYTE buffer[128];
        ssize_t len = recv(sock, buffer, 128, 0);
        if (len == 0 || len == SOCKET_ERROR)
            break;

        printf("%s\n", buffer);
    }

    exit(0);
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("syntax : echo-client <ip> <port>\n");
        printf("sample : echo-client 192.168.10.2 1234\n");
        return -1;
    }

    in_addr_t addr = inet_addr(argv[1]);
    int port = htons(argv[2]);

    printf("%d\n", port);

    if (addr == INADDR_NONE)
    {
        printf("INVALID ADDRESS\n");
        return -1;
    }
    
    if (65535 < port || port < 1)
    {
        printf("INVALID SOCKET PORT\n");
        return -1;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == INVALID_SOCKET)
    {
        printf("INVALID SOCKET\n");
        return -1;
    }

    struct sockaddr_in server_addr;

    ZeroMemory(&server_addr, sizeof(struct sockaddr_in));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = addr;
    server_addr.sin_port = port;

    int ret = connect(sock, (const struct sockaddr*)&server_addr, sizeof(server_addr));

    if (ret == SOCKET_ERROR)
    {
        printf("Connect Failed\n");
        return -1;
    }

    pthread_t RecvThreadId;

    pthread_create(&RecvThreadId, NULL, RecvThread, sock);
    pthread_detach(RecvThreadId);

    while (TRUE)
    {
        BYTE buffer[128];
        fgets(buffer, 127, stdin);
        buffer[strlen(buffer)] = NULL;
        ssize_t len = send(sock, buffer, 128, 0);

        if (len == 0 || len == SOCKET_ERROR)
            break;
    }

    close(sock);

    return 0;
}