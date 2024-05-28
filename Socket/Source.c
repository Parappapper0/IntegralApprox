#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 4096

DWORD WINAPI thread_accept(LPVOID arg);
int socket_send(SOCKET socket, const char* buffer, int length, int flags);
const char* get_mime_type(const char* path);

int main() {
    WSADATA wsa_data;
    int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (result != 0) {
        printf("C'%c stato un errore nello WSAStartup : %d\n", (char)138, result);
        return 1;
    }

    struct addrinfo* s_result = NULL, hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    result = getaddrinfo(NULL, "8080", &hints, &s_result); // Use port 8080 for testing
    if (result != 0) {
        printf("C'%c stato un errore nel getaddrinfo : %d\n", (char)138, result);
        WSACleanup();
        return 1;
    }

    SOCKET server_socket = socket(s_result->ai_family, s_result->ai_socktype, s_result->ai_protocol);
    if (server_socket == INVALID_SOCKET) {
        printf("C'%c stato un errore nel socket() : %ld\n", (char)138, WSAGetLastError());
        freeaddrinfo(s_result);
        WSACleanup();
        return 1;
    }

    result = bind(server_socket, s_result->ai_addr, (int)s_result->ai_addrlen);
    if (result == SOCKET_ERROR) {
        printf("C'%c stato un errore nel bind: %d\n", (char)138, WSAGetLastError());
        freeaddrinfo(s_result);
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(s_result);

    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
        printf("C'%c stato un errore nel listen: %ld\n", (char)138, WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    do {
        SOCKET* client_socket = (SOCKET*)malloc(sizeof(SOCKET));

        *client_socket = accept(server_socket, NULL, NULL);
        if (*client_socket == INVALID_SOCKET) {
            printf("C'%c stato un errore nell'accept: %d\n", (char)138, WSAGetLastError());
            closesocket(server_socket);
            free(client_socket);
            WSACleanup();
            return 1;
        }

        HANDLE thread_handle = CreateThread(NULL, 0, thread_accept, (void*)client_socket, 0, NULL);
        if (thread_handle == NULL) {
            printf("C'%c stato un errore nella creazione del thread: %d\n", (char)138, GetLastError());
            closesocket(*client_socket);
            free(client_socket);
        }
        else {
            CloseHandle(thread_handle);
        }

    } while (result >= 0);

    closesocket(server_socket);
    WSACleanup();
    return 0;
}

DWORD WINAPI thread_accept(LPVOID arg) {
    SOCKET client_socket = *(SOCKET*)arg;
    free(arg);

    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    char sendbuf[DEFAULT_BUFLEN];
    int sendbuflen = DEFAULT_BUFLEN;
    int result;
    FILE* file;

    result = recv(client_socket, recvbuf, recvbuflen, 0);
    if (result > 0) {
        printf("Byte ricevuti: %d\n", result);
        if (result < DEFAULT_BUFLEN)
            recvbuf[result] = '\0'; // Ensure null-terminated string
        printf("Contenuto ricevuto: \n%s", recvbuf);

        if (strncmp(recvbuf, "GET", 3) == 0) {
            char* resource_start = strchr(recvbuf, ' ') + 1;
            char* resource_end = strchr(resource_start, ' ');
            __int64 resource_length = resource_end - resource_start;

            char* resource = (char*)malloc(resource_length + 1);
            strncpy_s(resource, resource_length + 1, resource_start, resource_length);
            resource[resource_length] = '\0';

            if (strcmp(resource, "/") == 0) {
                void* mem = realloc(resource, 12);
                if (mem != NULL)
                    resource = mem;
                strncpy_s(resource, 12, "/index.html", 12);
            }

            char filepath[256] = "../Angular/dist/integral-approx/browser";
            strncat_s(filepath, sizeof(filepath), resource, sizeof(filepath) - strlen(filepath) - 1);

            fopen_s(&file, filepath, "rb");
            if (file == NULL) {
                perror("Error opening file");
                snprintf(sendbuf, sendbuflen,
                    "HTTP/1.1 404 Not Found\r\n"
                    "Content-Type: text/html\r\n"
                    "Content-Length: 14\r\n"
                    "\r\n"
                    "404 Not Found");
                socket_send(client_socket, sendbuf, (int)strlen(sendbuf), 0);
            }
            else {
                fseek(file, 0, SEEK_END);
                long file_size = ftell(file);
                fseek(file, 0, SEEK_SET);

                char* file_content = (char*)malloc((size_t)file_size + 1);
                fread(file_content, 1, file_size, file);
                file_content[file_size] = '\0';

                fclose(file);

                snprintf(sendbuf, sendbuflen,
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: %s\r\n"
                    "Content-Length: %ld\r\n"
                    "\r\n",
                    get_mime_type(filepath), file_size);
                socket_send(client_socket, sendbuf, (int)strlen(sendbuf), 0);
                socket_send(client_socket, file_content, (int)file_size, 0);

                free(file_content);
            }

            free(resource);
        }
        else if (strncmp(recvbuf, "POST", 4) == 0 || recvbuf[0] == '{') {
           
            char* body = strstr(recvbuf, "\r\n\r\n");
            if (body)
                body += 4;
            
            if (body[0] == '\0') {
                recv(client_socket, recvbuf, recvbuflen, 0);
                if (result < DEFAULT_BUFLEN)
                    recvbuf[result] = '\0';
                printf("Contenuto(2) ricevuto: \n%s", recvbuf);

                char* end = strrchr(recvbuf, '}');
                int json_len = end - recvbuf + 1;

                char* json_content = (char*)malloc(json_len + 1);
                strncpy_s(json_content, json_len + 1, recvbuf, json_len);

                printf("Extracted JSON content:\n%s\n", json_content);
            }

            double a;
            double b;
            int n;
            char function[1024] = "";
            char method[22] = ""; //arbitrary

            // Parsing JSON
            char* context = NULL;
            char* token = strtok_s(recvbuf, ":,{}\"", &context);

            while (token != NULL) {
                if (strcmp(token, "bound A") == 0) {
                    token = strtok_s(NULL, ":,{}\"", &context);
                    a = strtod(token, NULL);
                }
                else if (strcmp(token, "bound B") == 0) {
                    token = strtok_s(NULL, ":,{}\"", &context);
                    b = strtod(token, NULL);
                }
                else if (strcmp(token, "n") == 0) {
                    token = strtok_s(NULL, ":,{}\"", &context);
                    n = atoi(token);
                }
                else if (strcmp(token, "expression") == 0) {
                    token = strtok_s(NULL, ":,{}\"", &context);
                    strcpy_s(function, sizeof(function), token);
                }
                else if (strcmp(token, "method") == 0) {
                    token = strtok_s(NULL, ":,{}\"", &context);
                    strcpy_s(method, sizeof(method), token);
                }
                token = strtok_s(NULL, ":,{}\"", &context);
            }

            int method_i = strncmp(method, "Rectangles", 10) == 0 ? 0 : strncmp(method, "Trapezoids", 10) == 0 ? 1 : 2;

            fopen_s(&file, "../Processor/function.h", "wb");
            if (file != 0) {
                fprintf(file, "#include <math.h>\ndouble f(double x) {return ");
                fprintf(file, function);
                fprintf(file, ";}");
                fclose(file);
            }
            system("gcc -O3 ../Processor/function.h");
            system("gcc -O3 ../Processor/helpers.h");
            system("gcc -O3 ../Processor/source.c");

            SetCurrentDirectory(L"../Processor");

            sprintf_s(function, sizeof(function), "a.exe %lf %lf %d %d", a, b, n, method_i);
            file = _popen(function, "r");
            fgets(sendbuf, sizeof(sendbuf), file);
            fgets(sendbuf, sizeof(sendbuf), file);

            double value;
            double time;

            sscanf_s(sendbuf, "%lf %lf", &value, &time);
            _pclose(file);

            snprintf(sendbuf, sendbuflen,
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: application/json\r\n"
                "\r\n"
                "{\"value\":\"%.10lf\", \"time\":\"%.10lf\"}", value, time);

            socket_send(client_socket, sendbuf, (int)strlen(sendbuf), 0);
            printf("Sent JSON response: %s\n", sendbuf); // Print the sent response for debugging
        }
    }
    else if (result == 0) {
        printf("Connection closing...\n");
    }
    else {
        printf("C'%c stato un errore nella ricezione: %d\n", (char)138, WSAGetLastError());
    }

    closesocket(client_socket);
    ExitThread(0);
}

int socket_send(SOCKET socket, const char* buffer, int length, int flags) {
    int send_result = send(socket, buffer, length, flags);
    if (send_result == SOCKET_ERROR) {
        printf("C'%c stato un errore nell'invio: %d\n", (char)138, WSAGetLastError());
        closesocket(socket);
        WSACleanup();
        return SOCKET_ERROR;
    }
    printf("Byte inviati: %d\n", send_result);
    return send_result;
}

const char* get_mime_type(const char* path) {
    const char* ext = strrchr(path, '.');
    if (!ext) return "application/octet-stream"; // Default binary type

    if (strcmp(ext, ".html") == 0) return "text/html";
    if (strcmp(ext, ".htm") == 0) return "text/html";
    if (strcmp(ext, ".css") == 0) return "text/css";
    if (strcmp(ext, ".js") == 0) return "application/javascript";
    if (strcmp(ext, ".json") == 0) return "application/json";
    if (strcmp(ext, ".jpg") == 0) return "image/jpeg";
    if (strcmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".gif") == 0) return "image/gif";
    if (strcmp(ext, ".bmp") == 0) return "image/bmp";
    if (strcmp(ext, ".ico") == 0) return "image/x-icon";
    if (strcmp(ext, ".pdf") == 0) return "application/pdf";
    if (strcmp(ext, ".txt") == 0) return "text/plain";
    if (strcmp(ext, ".xml") == 0) return "text/xml";
    if (strcmp(ext, ".jst") == 0) return "text/html";
    // Add more extensions as needed

    return "application/octet-stream"; // Default binary type
}
