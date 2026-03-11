/*
Polecenie:
(nieobowiązkowe) Kolejna modyfikacja: popraw program tak, aby i znaki '\n', i dwubajtowe sekwencje złożone ze znaku '\r' i następującego po nim znaku '\n' były traktowane jako terminatory linii.
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define BUF_SIZE 1024
#define REM_BUF_SIZE (BUF_SIZE * 2)

static ssize_t write_all(int fd, const char *buf, ssize_t len) {
    ssize_t total = 0;
    while (total < len) {
        ssize_t n = write(fd, buf + total, len - total);
        if (n == -1) {
            return -1;
        }
        total += n;
    }
    return total;
}

static int is_line_end(const char *buf, ssize_t i, ssize_t len) {
    if (buf[i] == '\n') {
        return 1;
    }
    if (buf[i] == '\r' && i + 1 < len && buf[i + 1] == '\n') {
        return 2;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source file> <destination file>\n", argv[0]);
        return 1;
    }

    int fd_src = open(argv[1], O_RDONLY);
    if (fd_src == -1) {
        perror("Error opening source file");
        return 1;
    }

    int fd_dst = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd_dst == -1) {
        perror("Error opening destination file");
        close(fd_src);
        return 1;
    }

    char rem_buf[REM_BUF_SIZE];
    ssize_t rem_len = 0;
    int line_num = 1;
    ssize_t bytes_read;

    while ((bytes_read = read(fd_src, rem_buf + rem_len, REM_BUF_SIZE - rem_len)) > 0) {
        rem_len += bytes_read;
        const char *buf = rem_buf;
        ssize_t len = rem_len;
        ssize_t line_start = 0;

        for (ssize_t i = 0; i < len; i++) {
            int term_len = is_line_end(buf, i, len);
            if (term_len > 0) {
                ssize_t line_len = i - line_start + term_len;
                if (line_num % 2 == 1) {
                    if (write_all(fd_dst, buf + line_start, line_len) == -1) {
                        perror("Error writing to destination file");
                        close(fd_src);
                        close(fd_dst);
                        return 1;
                    }
                }
                line_num++;
                line_start = i + term_len;
                i += term_len - 1;
            }
        }

        rem_len = len - line_start;
        if (rem_len > 0) {
            memmove(rem_buf, buf + line_start, rem_len);
        }
    }

    if (bytes_read == -1) {
        perror("Error reading from source file");
        close(fd_src);
        close(fd_dst);
        return 1;
    }

    if (rem_len > 0 && line_num % 2 == 1) {
        if (write_all(fd_dst, rem_buf, rem_len) == -1) {
            perror("Error writing to destination file");
            close(fd_src);
            close(fd_dst);
            return 1;
        }
    }

    if (close(fd_src) == -1) {
        perror("Error closing source file");
        close(fd_dst);
        return 1;
    }

    if (close(fd_dst) == -1) {
        perror("Error closing destination file");
        return 1;
    }
    return 0;
}
