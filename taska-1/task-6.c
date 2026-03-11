/*
Polecenie:
Zaimplementuj program kopiujący dane z pliku do pliku przy pomocy powyższych funkcji. Zakładamy, że nazwy plików są podawane przez użytkownika jako argumenty programu (tzn. będą dostępne w tablicy argv). Zwróć szczególną uwagę na obsługę błędów — każde wywołanie funkcji we-wy musi być opatrzone testem sprawdzającym, czy zakończyło się ono sukcesem, czy porażką.

Funkcje POSIX zwracają -1 aby zasygnalizować wystąpienie błędu, i dodatkowo zapisują w globalnej zmiennej errno kod wskazujący przyczynę wystąpienia błędu (na dysku nie ma pliku o takiej nazwie, brak wystarczających praw dostępu, itd.). Polecam Państwa uwadze pomocniczą funkcję perror, która potrafi przetłumaczyć ten kod na zrozumiały dla człowieka komunikat i wypisać go na ekranie.
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define BUF_SIZE 1024

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

    char buffer[BUF_SIZE];
    ssize_t bytes_read, bytes_written;
    while ((bytes_read = read(fd_src, buffer, BUF_SIZE)) > 0) {
        ssize_t total_written = 0;
        while (total_written < bytes_read) {
            bytes_written = write(fd_dst, buffer + total_written, bytes_read - total_written);
            if (bytes_written == -1) {
                perror("Error writing to destination file");
                close(fd_src);
                close(fd_dst);
                return 1;
            }
            total_written += bytes_written;
        }
    }

    if (bytes_read == -1) {
        perror("Error reading from source file");
        close(fd_src);
        close(fd_dst);
        return 1;
    }

    if (close(fd_src) == -1) {
        perror("Error closing source file");
        // Continue to try closing the destination file
        close(fd_dst);
        return 1;
    }

    if (close(fd_dst) == -1) {
        perror("Error closing destination file");
        return 1;
    }
    return 0;
}