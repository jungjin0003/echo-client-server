#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

typedef struct _ClientList
{
    SOCKET sock;
    void *next;
} ClientList;

BOOL BroadCast = FALSE;
BOOL Echo = FALSE;
ClientList *clientlist = NULL;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void AddClient(SOCKET sock)
{
    ClientList *list = clientlist;
    pthread_mutex_lock(&mutex);
    while (list->next)
    {
        list = list->next;
    }
    
    list->sock = sock;
    list->next = malloc(sizeof(ClientList));

    ZeroMemory(list->next, sizeof(ClientList));
    pthread_mutex_unlock(&mutex);
}

void RemoveClient(SOCKET sock)
{
    ClientList *prev = NULL;
    ClientList *cur = clientlist;
    pthread_mutex_lock(&mutex);
    while (cur->sock != sock)
    {
        prev = cur;
        cur = cur->next;
    }

    if (prev == NULL)
    {
        clientlist = cur->next;
        free(cur);
    }
    else
    {
        prev->next = cur->next;
        free(cur);
    }
    pthread_mutex_unlock(&mutex);
}

void BroadCastSend(BYTE *buffer, size_t len)
{
    ClientList *list = clientlist;
    pthread_mutex_lock(&mutex);
    while (list->next)
    {
        send(list->sock, buffer, len, 0);
        list = list->next;
    }
    pthread_mutex_unlock(&mutex);
}

void ClientThread(SOCKET* client)
{
    AddClient(client);
    while (TRUE)
    {
        BYTE buffer[512] = { 0 ,};
        ssize_t len = recv(client, buffer, 512, 0);

        if (len <= 0)
            break;

        if (Echo)
            if (BroadCast)
                BroadCastSend(buffer, len);
            else
                send(client, buffer, len, 0);

        printf("%s\n", buffer);
    }
    RemoveClient(client);
}

void param(int argc, char *argv[])
{
    for (int i = 2; i < argc; i++)
    {
        if (strcmp(argv[i], "-e") == 0)
            Echo = TRUE;
        if (strcmp(argv[i], "-b") == 0)
            BroadCast = TRUE;
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("syntax : echo-server <port > [-e[-b]]\n");
        printf("sample : echo-server 1234 -e -b\n");
        return -1;
    }

    int port = atoi(argv[1]);

    if (65535 < port || port < 1)
    {
        printf("INVALID SOCKET PORT\n");
        return -1;
    }

    if (argc > 2)
        param(argc, argv);

    SOCKET listner = socket(AF_INET, SOCK_STREAM, 0);

    if (listner == INVALID_SOCKET)
    {
        printf("INVALID SOCKET\n");
        return -1;
    }

    struct sockaddr_in host_addr;

    ZeroMemory(&host_addr, sizeof(struct sockaddr_in));

    host_addr.sin_family = AF_INET;
    host_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    host_addr.sin_port = htons(port);

    int ret = bind(listner, (const struct sockaddr *)&host_addr, sizeof(host_addr));

    if (ret == SOCKET_ERROR)
    {
        printf("bind Failed\n");
        return -1;
    }

    ret = listen(listner, 5);

    if (ret == SOCKET_ERROR)
    {
        printf("listen Failed\n");
        return -1;
    }

    clientlist = malloc(sizeof(ClientList));
    ZeroMemory(clientlist, sizeof(ClientList));

    while (1)
    {
        pthread_t ClientThreadId;
        struct sockaddr_in client_addr;

        ZeroMemory(&client_addr, sizeof(struct sockaddr_in));

        int addrlen = sizeof(struct sockaddr_in);

        SOCKET client = accept(listner, (struct sockaddr *)&client_addr, &addrlen);

        printf("Connected : %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        pthread_create(&ClientThreadId, NULL, ClientThread, client);
        pthread_detach(ClientThreadId);
    }
}