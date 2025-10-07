#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsa;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server, client;
    int c;
    char buffer[1024];

    printf("Inicializando Winsock...\n");
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        printf("Falha na inicialização.\n");
        return 1;
    }

    // Cria socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        printf("Erro ao criar socket.\n");
        WSACleanup();
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8080);

    // Associa o socket à porta
    if (bind(server_socket, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Erro no bind.\n");
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    // Escutando conexões
    listen(server_socket, 3);
    printf("Servidor aguardando conexões na porta 8080...\n");

    c = sizeof(struct sockaddr_in);

    client_socket = accept(server_socket, (struct sockaddr*)&client, &c);
    if (client_socket == INVALID_SOCKET) {
        printf("Erro no accept.\n");
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    printf("Cliente conectado!\n");

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int recv_size = recv(client_socket, buffer, sizeof(buffer), 0);
        if (recv_size <= 0) {
            printf("Cliente desconectado.\n");
            break;
        }

        printf("Cliente: %s\n", buffer);

        if (strcmp(buffer, "sair") == 0) {
            printf("Encerrando conexão...\n");
            break;
        }

        send(client_socket, "Mensagem recebida!\n", 20, 0);
    }

    closesocket(client_socket);
    closesocket(server_socket);
    WSACleanup();

    return 0;
}
