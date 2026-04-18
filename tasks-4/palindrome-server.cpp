#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <cstring>
#include <cctype>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define PORT 2020
#define MAX_LINE 1024
#define MAX_EVENTS 64

struct ClientData {
    string input;
    string output;
};

map<int, ClientData> clients;

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


int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    int yes = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }

    if (listen(server_fd, 128) < 0) {
        perror("listen");
        return 1;
    }

    int flags = fcntl(server_fd, F_GETFL, 0);
    fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);

    int epfd = epoll_create1(0);
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &ev);

    vector<struct epoll_event> events(MAX_EVENTS);

    while (true) {
        int n = epoll_wait(epfd, events.data(), MAX_EVENTS, -1);

        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;

            if (fd == server_fd) {
                    struct sockaddr_in client_addr;
                    socklen_t len = sizeof(client_addr);
                    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &len);
                    if (client_fd < 0)
                        break;

                    int fl = fcntl(client_fd, F_GETFL, 0);
                    fcntl(client_fd, F_SETFL, fl | O_NONBLOCK);

                    struct epoll_event cev;
                    cev.events = EPOLLIN;
                    cev.data.fd = client_fd;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &cev);

                    clients[client_fd] = ClientData();
                continue;
            }

            if (events[i].events & (EPOLLERR | EPOLLHUP)) {
                epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                close(fd);
                clients.erase(fd);
                continue;
            }

            if (events[i].events & EPOLLIN) {
                char buf[4096];
                bool disconnected = false;

                    ssize_t r = read(fd, buf, sizeof(buf));
                    if (r < 0) {
                        if (errno == EAGAIN)
                            break;
                        disconnected = true;
                        break;
                    }
                    if (r == 0) {
                        disconnected = true;
                        break;
                    }
                    clients[fd].input.append(buf, r);

                if (disconnected) {
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                    close(fd);
                    clients.erase(fd);
                    continue;
                }

                    size_t pos = clients[fd].input.find("\r\n");
                    if (pos == string::npos) {
                        if (clients[fd].input.size() > MAX_LINE) {
                            epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                            close(fd);
                            clients.erase(fd);
                        }
                        break;
                    }

                    string line = clients[fd].input.substr(0, pos);
                    clients[fd].input.erase(0, pos + 1);

                    bool bad = false;
                    for (int j = 0; j < (int)line.size(); j++) {
                        if ((unsigned char)line[j] < 0x20 || (unsigned char)line[j] > 0x7e) {
                            bad = true;
                            break;
                        }
                    }
                    if (bad) {
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                        close(fd);
                        clients.erase(fd);
                        break;
                    }

                    clients[fd].output += process(line);

                if (clients.count(fd) && !clients[fd].output.empty()) {
                    struct epoll_event mod;
                    mod.events = EPOLLIN | EPOLLOUT;
                    mod.data.fd = fd;
                    epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &mod);
                }
            }

            if (events[i].events & EPOLLOUT) {
                if (!clients.count(fd))
                    continue;

                while (!clients[fd].output.empty()) {
                    int w = write(fd, clients[fd].output.c_str(), clients[fd].output.size());
                    if (w < 0) {
                        if (errno == EAGAIN)
                            break;
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                        close(fd);
                        clients.erase(fd);
                        break;
                    }
                    clients[fd].output.erase(0, w);
                }

                if (clients.count(fd) && clients[fd].output.empty()) {
                    struct epoll_event mod;
                    mod.events = EPOLLIN;
                    mod.data.fd = fd;
                    epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &mod);
                }
            }
        }
    }

    close(epfd);
    close(server_fd);
    return 0;
}