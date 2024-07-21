#include <winsock2.h>
#include <windows.h>
#include <stdio.h>

int main()
{
    WSADATA wsadata;
    if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }

    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) {
        printf("Socket creation failed\n");
        WSACleanup();
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);

    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        printf("Bind failed\n");
        closesocket(s);
        WSACleanup();
        return 1;
    }

    if (listen(s, 10) == SOCKET_ERROR) {
        printf("Listen failed\n");
        closesocket(s);
        WSACleanup();
        return 1;
    }

    printf("Listening on port 8080...\n");

    SOCKET client = accept(s, NULL, NULL);
    if (client == INVALID_SOCKET) {
        printf("Accept failed\n");
        closesocket(s);
        WSACleanup();
        return 1;
    }

    char request[1024] = {0};
    recv(client, request, sizeof(request) - 1, 0);

    // Check for "GET /" request
    if (memcmp(request, "GET / ", 6) == 0) {
        FILE* f = fopen("index.html", "rb");
        if (f == NULL) {
            printf("File not found\n");
            const char* not_found_response = 
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Length: 0\r\n"
                "\r\n";
            send(client, not_found_response, strlen(not_found_response), 0);
        } else {
            fseek(f, 0, SEEK_END);
            long filesize = ftell(f);
            fseek(f, 0, SEEK_SET);

            char* filedata = (char*)malloc(filesize);
            fread(filedata, 1, filesize, f);
            fclose(f);

            char response[1024];
            int response_length = snprintf(response, sizeof(response),
                "HTTP/1.1 200 OK\r\n"
                "Content-Length: %ld\r\n"
                "Content-Type: text/html\r\n"
                "\r\n", filesize);

            send(client, response, response_length, 0);
            send(client, filedata, filesize, 0);

            free(filedata);
        }
    }

    closesocket(client);
    closesocket(s);
    WSACleanup();

    return 0;
}