#include "../srcs/uprotocol.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>

#include <thread>
#include <mutex>
#include <vector>
#include <iostream>
#include <chrono>
#include <atomic>
#include <random>
#include <cstring>
#include <algorithm> // <--- needed for std::remove

static const char *SOCK_PATH = "/tmp/uprotocol_speed.sock";

std::atomic<bool> running{true};
std::mutex clients_mtx;
std::vector<int> clients;

void remove_and_close_client(int fd) {
    close(fd);
    std::lock_guard<std::mutex> lk(clients_mtx);
    // use std::remove (erase-remove idiom)
    clients.erase(std::remove(clients.begin(), clients.end(), fd), clients.end());
}

void accept_loop(int listen_fd) {
    while (running.load()) {
        int cfd = accept(listen_fd, nullptr, nullptr);
        if (cfd < 0) {
            if (errno == EINTR) continue;
            perror("accept");
            break;
        }
        // make send calls blocking (simple)
        {
            std::lock_guard<std::mutex> lk(clients_mtx);
            clients.push_back(cfd);
            std::cerr << "Client connected: fd=" << cfd << " (total=" << clients.size() << ")\n";
        }
    }
}

void broadcast_speed(double speed_m_s) {
    // payload: uint64_t timestamp_ms (network order) + double speed (raw bytes)
    uint64_t ts_ms = (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::system_clock::now().time_since_epoch())
                         .count();
    uint64_t ts_be = htonll(ts_ms);
    uint32_t payload_len = sizeof(ts_be) + sizeof(double);

    std::vector<uint8_t> buf(UPROTO_HEADER_SIZE + payload_len);
    UProtoHeader hdr;
    pack_header(hdr, MSG_SPEED, payload_len);
    // copy header (header layout is packed)
    memcpy(buf.data(), &hdr, UPROTO_HEADER_SIZE);
    // copy payload: timestamp then double
    memcpy(buf.data() + UPROTO_HEADER_SIZE, &ts_be, sizeof(ts_be));
    // copy double bytes (no byte-order conversion for double here)
    memcpy(buf.data() + UPROTO_HEADER_SIZE + sizeof(ts_be), &speed_m_s, sizeof(double));

    std::lock_guard<std::mutex> lk(clients_mtx);
    for (int fd : std::vector<int>(clients.begin(), clients.end())) {
        ssize_t w = send(fd, buf.data(), buf.size(), 0);
        if (w < 0) {
            std::cerr << "send to fd " << fd << " failed: " << strerror(errno) << "\n";
            remove_and_close_client(fd);
        }
    }
}

int main(int argc, char **argv) {
    (void)argc; (void)argv;
    // remove existing socket path
    unlink(SOCK_PATH);

    int listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (listen_fd < 0) { perror("socket"); return 1; }

    sockaddr_un sun{};
    sun.sun_family = AF_UNIX;
    strncpy(sun.sun_path, SOCK_PATH, sizeof(sun.sun_path)-1);

    if (bind(listen_fd, (sockaddr*)&sun, sizeof(sun)) < 0) { perror("bind"); close(listen_fd); return 1; }
    if (listen(listen_fd, 10) < 0) { perror("listen"); close(listen_fd); return 1; }
    std::cerr << "sensor_daemon listening on " << SOCK_PATH << "\n";

    std::thread accept_thread(accept_loop, listen_fd);

    // Simulate sensor: generate speeds using a simple pattern
    std::mt19937_64 rng((unsigned)time(nullptr));
    std::uniform_real_distribution<double> dist(0.0, 30.0); // 0..30 m/s (~0..108 km/h)

    while (running.load()) {
        double speed = dist(rng);
        broadcast_speed(speed);
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 10 Hz
    }

    // cleanup
    close(listen_fd);
    accept_thread.join();
    {
        std::lock_guard<std::mutex> lk(clients_mtx);
        for (int fd : clients) close(fd);
        clients.clear();
    }
    unlink(SOCK_PATH);
    return 0;
}