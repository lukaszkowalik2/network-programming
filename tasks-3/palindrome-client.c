#include <arpa/inet.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define MAX_UDP 65507
#define TIMEOUT_SEC 10

static void close_or_warn(int fd, const char *what) {
    if (close(fd) < 0) {
        perror(what);
    }
}

static int is_valid_response(const char *buf, ssize_t len) {
    if (len == 5 && memcmp(buf, "ERROR", 5) == 0) return 1;

    int saw_slash = 0;
    for (ssize_t i = 0; i < len; i++) {
        if (buf[i] == '/') {
            if (saw_slash) return 0;
            saw_slash = 1;
        } else if (!isdigit((unsigned char)buf[i])) {
            return 0;
        }
    }
    return saw_slash;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Użycie: %s <adres_ip> <port>\n", argv[0]);
        return 1;
    }

    const char *ip_str = argv[1];
    int port = atoi(argv[2]);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Nieprawidłowy port: %s\n", argv[2]);
        return 1;
    }

    struct sockaddr_in srv_addr;
    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons((uint16_t)port);
    if (inet_pton(AF_INET, ip_str, &srv_addr.sin_addr) != 1) {
        fprintf(stderr, "Nieprawidłowy adres IP: %s\n", ip_str);
        return 1;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    struct timeval tv;
    tv.tv_sec = TIMEOUT_SEC;
    tv.tv_usec = 0;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("setsockopt");
        close_or_warn(sock, "close");
        return 1;
    }

    unsigned char buf[MAX_UDP];
    ssize_t query_len = read(STDIN_FILENO, buf, sizeof(buf));
    if (query_len < 0) {
        perror("read");
        close_or_warn(sock, "close");
        return 1;
    }

    /* Strip trailing newline so piping works: echo "abc" | ./palindrome-client ... */
    while (query_len > 0 && (buf[query_len - 1] == '\n' || buf[query_len - 1] == '\r')) {
        query_len--;
    }

    if (sendto(sock, buf, (size_t)query_len, 0,
               (struct sockaddr *)&srv_addr, sizeof(srv_addr)) < 0) {
        perror("sendto");
        close_or_warn(sock, "close");
        return 1;
    }

    char resp[256];
    ssize_t n = recvfrom(sock, resp, sizeof(resp) - 1, 0, NULL, NULL);
    if (n < 0) {
        perror("recvfrom (timeout?)");
        close_or_warn(sock, "close");
        return 1;
    }

    /* Strip trailing \r\n from server response */
    while (n > 0 && (resp[n - 1] == '\n' || resp[n - 1] == '\r')) {
        n--;
    }
    resp[n] = '\0';

    if (is_valid_response(resp, n)) {
        printf("%s\n", resp);
    } else {
        fprintf(stderr, "Nieprawidłowa odpowiedź serwera: ");
        for (ssize_t i = 0; i < n; i++) {
            unsigned char c = (unsigned char)resp[i];
            if (isprint(c)) {
                fputc(c, stderr);
            } else {
                fprintf(stderr, "\\x%02x", c);
            }
        }
        fputc('\n', stderr);
        close_or_warn(sock, "close");
        return 1;
    }

    close_or_warn(sock, "close");
    return 0;
}
