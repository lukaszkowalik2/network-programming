#include <arpa/inet.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_UDP 65507


static void close_or_warn(int fd, const char *what) {
    if (close(fd) < 0) {
        perror(what);
    }
}

static int is_palindrome_word(const char *start, const char *end_inclusive) {
    const char *lo = start;
    const char *hi = end_inclusive;
    while (lo < hi) {
        if (tolower((unsigned char)*lo) != tolower((unsigned char)*hi)) {
            return 0;
        }
        lo++;
        hi--;
    }
    return 1;
}

static int validate_and_count(const unsigned char *buf, size_t len, int *out_total, int *out_pal) {
    if (len == 0) {
        *out_total = 0;
        *out_pal = 0;
        return 1;
    }

    if (buf[0] == ' ' || buf[len - 1] == ' ') return 0;

    int total = 0;
    int pals = 0;
    size_t start = 0;

    for (size_t i = 0; i <= len; i++) {
        // Forbidden characters
        if (i < len && buf[i] != ' ' && !isalpha(buf[i])) return 0;

        // Forbidden double spaces
        if (i < len - 1 && buf[i] == ' ' && buf[i + 1] == ' ') return 0;

        // End of word
        if (i == len || buf[i] == ' ') {
            total++;
            if (is_palindrome_word((const char*)&buf[start], (const char*)&buf[i - 1])) {
                pals++;
            }
            start = i + 1;
        }
    }

    *out_total = total;
    *out_pal = pals;
    return 1;
}

int main(int argc, char *argv[]) {
    int port = (argc > 1) ? atoi(argv[1]) : 2020;

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close_or_warn(sock, "close");
        return 1;
    }

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        unsigned char buf[MAX_UDP];

        ssize_t r = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *)&client_addr, &addr_len);
        if (r < 0) {
            perror("recvfrom");
            continue;
        }

        if (r >= 2 && buf[r-2] == '\r' && buf[r-1] == '\n') r -= 2;
        else if (r >= 1 && buf[r-1] == '\n') r -= 1;

        char out[64];
        const char *reply;

        int total = 0, pals = 0;
        if (validate_and_count(buf, (size_t)r, &total, &pals)) {
            snprintf(out, sizeof(out), "%d/%d", pals, total);
            reply = out;
        } else {
            reply = "ERROR";
        }

        if (sendto(sock, reply, strlen(reply), 0, (struct sockaddr *)&client_addr, addr_len) < 0) {
            perror("sendto");
        }
    }

    return 0;
}