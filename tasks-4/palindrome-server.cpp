#include <string>
#include <map>
#include <vector>
#include <cstring>
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <csignal>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define PORT 2020
#define MAX_LINE 1024
#define MAX_EVENTS 64
#define READ_CHUNK 4096

struct ClientData {
    std::string input;
    std::string output;
    bool shutdown_after_flush = false;
};

static std::map<int, ClientData> clients;
static int epfd = -1;

static void close_or_warn(int fd, const char *what) {
    if (close(fd) < 0) {
        perror(what);
    }
}

static bool is_palindrome_word(const std::string& word) {
    size_t lo = 0, hi = word.size() - 1;
    while (lo < hi) {
        if (std::tolower(static_cast<unsigned char>(word[lo])) !=
            std::tolower(static_cast<unsigned char>(word[hi]))) {
            return false;
        }
        ++lo;
        --hi;
    }
    return true;
}

static std::string process_line(const std::string &line) {
    if (line.empty()) return "0/0\r\n";
    if (line.front() == ' ' || line.back() == ' ') return "ERROR\r\n";

    size_t len = line.size();
    int total = 0, pals = 0;
    size_t start = 0;

    for (size_t i = 0; i <= len; ++i) {
        if (i < len) {
            if (line[i] != ' ' && !std::isalpha(static_cast<unsigned char>(line[i])))
                return "ERROR\r\n";
            if (i + 1 < len && line[i] == ' ' && line[i + 1] == ' ')
                return "ERROR\r\n";
        }

        if (i == len || line[i] == ' ') {
            ++total;
            if (is_palindrome_word(line.substr(start, i - start))) ++pals;
            start = i + 1;
        }
    }

    return std::to_string(pals) + "/" + std::to_string(total) + "\r\n";
}

static void close_client(int fd) {
    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
    close_or_warn(fd, "close client");
    clients.erase(fd);
}

static bool extract_lines(int fd) {
    ClientData &c = clients[fd];
    while (true) {
        size_t pos = c.input.find("\r\n");
        if (pos == std::string::npos) {
            if (c.input.size() > MAX_LINE) return false;
            return true;
        }

        std::string line = c.input.substr(0, pos);
        c.input.erase(0, pos + 2);

        for (unsigned char ch : line) {
            if (ch < 0x20 || ch > 0x7e) return false;
        }

        c.output += process_line(line);
    }
}

static bool try_write(int fd) {
    ClientData &c = clients[fd];
    while (!c.output.empty()) {
        ssize_t w = write(fd, c.output.data(), c.output.size());
        if (w < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) return true;
            if (errno == EINTR) continue;
            return false;
        }
        if (w == 0) return false;
        c.output.erase(0, (size_t)w);
    }
    return true;
}

static void set_nonblock(int fd) {
    int fl = fcntl(fd, F_GETFL, 0);
    if (fl >= 0) fcntl(fd, F_SETFL, fl | O_NONBLOCK);
}

static void handle_accept(int server_fd) {
    while (true) {
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &len);
        if (client_fd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) return;
            if (errno == EINTR) continue;
            return;
        }

        set_nonblock(client_fd);

        struct epoll_event cev;
        cev.events = EPOLLIN;
        cev.data.fd = client_fd;
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &cev) < 0) {
            close_or_warn(client_fd, "close client");
            continue;
        }

        clients[client_fd];
    }
}

static void handle_client(int fd, uint32_t events) {
    if (events & EPOLLIN) {
        char buf[READ_CHUNK];
        while (true) {
            ssize_t r = read(fd, buf, sizeof(buf));
            if (r > 0) {
                clients[fd].input.append(buf, (size_t)r);
                continue;
            }
            if (r == 0) {
                clients[fd].shutdown_after_flush = true;
                break;
            }
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            if (errno == EINTR) continue;
            close_client(fd);
            return;
        }

        if (!extract_lines(fd)) {
            close_client(fd);
            return;
        }
    }

    if (events & (EPOLLERR | EPOLLHUP) && !(events & EPOLLIN)) {
        close_client(fd);
        return;
    }

    if (!try_write(fd)) {
        close_client(fd);
        return;
    }

    ClientData &c = clients[fd];

    if (c.output.empty() && c.shutdown_after_flush) {
        close_client(fd);
        return;
    }
}

int main() {
    signal(SIGPIPE, SIG_IGN);

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
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close_or_warn(server_fd, "close server_fd");
        return 1;
    }

    if (listen(server_fd, 128) < 0) {
        perror("listen");
        close_or_warn(server_fd, "close server_fd");
        return 1;
    }

    set_nonblock(server_fd);

    epfd = epoll_create1(0);
    if (epfd < 0) {
        perror("epoll_create1");
        close_or_warn(server_fd, "close server_fd");
        return 1;
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &ev) < 0) {
        perror("epoll_ctl");
        close_or_warn(server_fd, "close server_fd");
        close_or_warn(epfd, "close epfd");
        return 1;
    }

    std::vector<struct epoll_event> events(MAX_EVENTS);

    while (true) {
        int n = epoll_wait(epfd, events.dat0x20a(), MAX_EVENTS, -1);
        if (n < 0) {
            if (errno == EINTR) continue;
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;
            uint32_t ev_mask = events[i].events;

            if (fd == server_fd) {
                handle_accept(server_fd);
            } else {
                handle_client(fd, ev_mask);
            }
        }
    }

    close_or_warn(server_fd, "close server_fd");
    close_or_warn(epfd, "close epfd");
    return 0;
}
