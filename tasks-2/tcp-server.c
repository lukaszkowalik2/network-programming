#include <arpa/inet.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static void close_or_warn(int fd, const char *what) {
    if (close(fd) < 0) {
        perror(what);
    }
}

int main(int argc, char *argv[]) {
    const char *bind_ip;
    const char *port_str;

    if (argc == 2) {
        bind_ip = "0.0.0.0";
        port_str = argv[1];
    } else if (argc == 3) {
        bind_ip = argv[1];
        port_str = argv[2];
    } else {
        fprintf(stderr,
                "Użycie: %s <port>\n"
                "       %s <adres_ip> <port>\n",
                argv[0], argv[0]);
        return 1;
    }

    int port = atoi(port_str);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Nieprawidłowy port: %s\n", port_str);
        return 1;
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    addr.sin_addr.s_addr = inet_addr(bind_ip);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close_or_warn(server_fd, "close");
        return 1;
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen");
        close_or_warn(server_fd, "close");
        return 1;
    }

    printf("Serwer TCP nasłuchuje na %s:%d...\n", bind_ip, port);

    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        const char *msg = "Hello, world!\r\n";
        ssize_t w = write(client_fd, msg, strlen(msg));
        if (w < 0) {
            perror("write");
        } else if ((size_t)w != strlen(msg)) {
            fprintf(stderr, "write: zapisano tylko %zd z %zu bajtów\n", w, strlen(msg));
        }

        close_or_warn(client_fd, "close (client_fd)");
    }

    close_or_warn(server_fd, "close (server_fd)");
    return 0;
}
