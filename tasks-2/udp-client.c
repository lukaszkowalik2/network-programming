#include <arpa/inet.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
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

    ssize_t s = sendto(sock, "", 0, 0, (struct sockaddr *)&addr, sizeof(addr));
    if (s < 0) {
        perror("sendto");
        close_or_warn(sock, "close");
        return 1;
    }

    char buffer[256];
    ssize_t n = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, NULL, NULL);
    if (n < 0) {
        perror("recvfrom");
        close_or_warn(sock, "close");
        return 1;
    }

    buffer[n] = '\0';
    printf(buffer);

    close_or_warn(sock, "close");
    return 0;
}
