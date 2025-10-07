#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>

#pragma comment(lib, "ws2_32.lib")

unsigned __stdcall recv_handler(void* pSocket) {
    SOCKET sock = (SOCKET)pSocket;
    char server_reply[1024];
    int recv_size;

    while (1) {
        memset(server_reply, 0, sizeof(server_reply));
        recv_size = recv(sock, server_reply, sizeof(server_reply) - 1, 0);

        if (recv_size <= 0) {
            printf("\rServidor desconectado. Pressione Enter para sair.\n");
            break;
        }

        server_reply[recv_size] = '\0';

        printf("\r%s\n", server_reply);
        printf("Você: ");
        fflush(stdout);
    }
    return 0;
}

int main() {
    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in serv_addr;
    char message[1024];

    char userName[50];
    char buffermsg[1200];

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Erro ao iniciar Winsock\n");
        return 1;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("Erro ao criar socket\n");
        WSACleanup();
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Falha na conexão\n");
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    printf("Conectado ao servidor!\n");
    printf("Digite seu nome de usuário: ");
    fgets(userName, sizeof(userName), stdin);
    userName[strcspn(userName, "\n")] = '\0';

    _beginthreadex(NULL, 0, recv_handler, (void*)sock, 0, NULL);

    while (1) {
        printf("Você: ");
        fgets(message, sizeof(message), stdin);
        message[strcspn(message, "\n")] = '\0';

        if (strcmp(message, "sair") == 0) {
            snprintf(buffermsg, sizeof(buffermsg), "%s: sair", userName);
            send(sock, buffermsg, strlen(buffermsg), 0);
            break;
        }

        snprintf(buffermsg, sizeof(buffermsg), "%s: %s", userName, message);
        send(sock, buffermsg, strlen(buffermsg), 0);
        
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}