#include <arpa/inet.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
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

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    addr.sin_addr.s_addr = inet_addr(bind_ip);

    if (inet_pton(AF_INET, bind_ip, &addr.sin_addr) != 1) {
        fprintf(stderr, "Nieprawidłowy adres IP: %s\n", bind_ip);
        close_or_warn(sock, "close");
        return 1;
    }

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close_or_warn(sock, "close");
        return 1;
    }

    unsigned char buffer[256];
    ssize_t n;
    while ((n = read(sock, buffer, sizeof(buffer))) > 0) {
        for (ssize_t i = 0; i < n; i++) {
            unsigned char c = buffer[i];
            if (isprint(c) || c == '\n' || c == '\r' || c == '\t') {
                putchar(c);
            }
        }
    }
    if (n < 0) {
        perror("read");
        close_or_warn(sock, "close");
        return 1;
    }

    close_or_warn(sock, "close");
    return 0;
}
