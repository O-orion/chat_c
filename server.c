#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <process.h>

#pragma comment(lib, "ws2_32.lib")

#define MAX_CLIENTS 10

SOCKET client_sockets[MAX_CLIENTS];
HANDLE mutex_client_list;
int client_count = 0;

// add socket to client global list
void add_client(SOCKET sock) {
    WaitForSingleObject(mutex_client_list, INFINITE);

    if (client_count < MAX_CLIENTS) {
        client_sockets[client_count] = sock;
        client_count++;
        printf("Client adicionado. total de clients: %d\n", client_count);
    } else {
        printf("Limite de clients atingido..\n");
    }

    ReleaseMutex(mutex_client_list);
}

// remove socket from client global list
void remove_client(SOCKET sock) {
    WaitForSingleObject(mutex_client_list, INFINITE);

    for(int i=0; i < client_count; i++) {
        if (client_sockets[i] == sock) {
            client_sockets[i] = client_sockets[client_count - 1];
            client_count--;
            break;
        }
    }
    printf("Client removido.");
    ReleaseMutex(mutex_client_list);
}

// Broadcast message to all clients
void broadcast_message(char *message, int len, SOCKET sender_sock) {
    WaitForSingleObject(mutex_client_list, INFINITE);

    for(int i=0; i < client_count; i++) {
        SOCKET target_sock = client_sockets[i];
        send(target_sock, message, len, 0);
    }
    ReleaseMutex(mutex_client_list);
}

// Function to thread for each client
unsigned __stdcall client_handle(void* pClientSocket) {
    SOCKET client_sock = (SOCKET)pClientSocket;
    char buffer[1024];

    while(1) {
        memset(buffer, 0, sizeof(buffer));
        int recv_size = recv(client_sock, buffer, sizeof(buffer), 0);

        if(recv_size <= 0) {
            printf("Client desconectado.\n");
            break;
        }

        if(strstr(buffer, ": sair") != NULL || strcmp(buffer, "sair") == 0) {
            printf("Cliente pediu para sair. \n");
            break;
        }

        printf("Broadcast: %s\n", buffer);
        broadcast_message(buffer, recv_size, client_sock);

    }

    closesocket(client_sock);
    remove_client(client_sock);
    return 0;
}

int main() {
    WSADATA wsa;
    SOCKET server_socket; 
    struct sockaddr_in server, client;
    int c;
    SOCKET new_client_socket; 

    printf("Inicializando Winsock...\n");
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        printf("Falha na inicialização.\n");
        return 1;
    }

   
    mutex_client_list = CreateMutex(NULL, FALSE, NULL);
    if (mutex_client_list == NULL) {
        printf("Erro ao criar mutex.\n");
        WSACleanup();
        return 1;
     }

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) { 
        printf("Não foi possível criar o socket.\n");
        WSACleanup();
        return 1;
     }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8080);

    if (bind(server_socket, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) { 
        printf("Bind falhou.\n");
        closesocket(server_socket);
        WSACleanup();
        return 1;
     }


    listen(server_socket, 5);
    printf("Servidor aguardando conexões na porta 8080...\n");

    while (1) {
        c = sizeof(struct sockaddr_in);
        
        new_client_socket = accept(server_socket, (struct sockaddr*)&client, &c);
        
        if (new_client_socket == INVALID_SOCKET) {
            printf("Erro no accept. Ignorando...\n");
            continue;
        }

        printf("Novo cliente conectado! Iniciando thread...\n");
        
        add_client(new_client_socket);
        
        _beginthreadex(
            NULL, 0, client_handle, (void*)new_client_socket, 0, NULL
        );
        
    }
    
    CloseHandle(mutex_client_list);
    closesocket(server_socket);
    WSACleanup();

    return 0;
}