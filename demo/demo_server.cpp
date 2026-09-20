#include <arpa/inet.h>
#include <csignal>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

namespace fs = std::filesystem;

namespace {
volatile std::sig_atomic_t running = 1;

void stop_server(int) { running = 0; }

std::string mime_type(const fs::path& path) {
    const auto extension = path.extension().string();
    if (extension == ".html") return "text/html; charset=utf-8";
    if (extension == ".js") return "text/javascript; charset=utf-8";
    if (extension == ".css") return "text/css; charset=utf-8";
    if (extension == ".svg") return "image/svg+xml";
    if (extension == ".png") return "image/png";
    if (extension == ".jpg" || extension == ".jpeg") return "image/jpeg";
    if (extension == ".woff2") return "font/woff2";
    return "application/octet-stream";
}

void reply(int client, int status, const std::string& reason, const std::string& content_type, const std::string& body) {
    const std::string header = "HTTP/1.1 " + std::to_string(status) + " " + reason + "\r\n"
        "Content-Type: " + content_type + "\r\n"
        "Content-Length: " + std::to_string(body.size()) + "\r\n"
        "Connection: close\r\n\r\n";
    (void)send(client, header.data(), header.size(), 0);
    (void)send(client, body.data(), body.size(), 0);
}

void serve_client(int client, const fs::path& root) {
    char buffer[4096]{};
    const auto received = recv(client, buffer, sizeof(buffer) - 1, 0);
    if (received <= 0) return;

    const std::string request(buffer, static_cast<size_t>(received));
    const auto request_end = request.find("\r\n");
    const std::string request_line = request.substr(0, request_end);
    const auto first_space = request_line.find(' ');
    const auto second_space = request_line.find(' ', first_space + 1);
    if (first_space == std::string::npos || second_space == std::string::npos || request_line.substr(0, first_space) != "GET") {
        reply(client, 405, "Method Not Allowed", "text/plain; charset=utf-8", "Only GET is supported\n");
        return;
    }

    std::string url_path = request_line.substr(first_space + 1, second_space - first_space - 1);
    const auto query_start = url_path.find_first_of("?#");
    if (query_start != std::string::npos) url_path.resize(query_start);
    if (url_path.empty() || url_path == "/") url_path = "/index.html";
    if (url_path.find("..") != std::string::npos) {
        reply(client, 400, "Bad Request", "text/plain; charset=utf-8", "Invalid path\n");
        return;
    }

    fs::path file = root / url_path.substr(1);
    if (!fs::is_regular_file(file)) file = root / "index.html";
    std::ifstream stream(file, std::ios::binary);
    if (!stream) {
        reply(client, 404, "Not Found", "text/plain; charset=utf-8", "Build the frontend first: make frontend\n");
        return;
    }
    const std::string body((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    reply(client, 200, "OK", mime_type(file), body);
}
}

int main(int argc, char* argv[]) {
    int port = 8088;
    fs::path root = "public";
    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];
        if (argument == "--port" && index + 1 < argc) port = std::stoi(argv[++index]);
        else if (argument == "--root" && index + 1 < argc) root = argv[++index];
        else if (argument == "--help") {
            std::cout << "Usage: demo_server [--port 8088] [--root public]\n";
            return 0;
        }
    }
    root = fs::absolute(root);
    if (!fs::is_regular_file(root / "index.html")) {
        std::cerr << "Frontend not found in " << root << ". Run make frontend first.\n";
        return 1;
    }

    std::signal(SIGINT, stop_server);
    std::signal(SIGTERM, stop_server);
    const int server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) { perror("socket"); return 1; }
    int reuse = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(static_cast<uint16_t>(port));
    if (bind(server, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0 || listen(server, 16) < 0) {
        perror("bind/listen");
        close(server);
        return 1;
    }
    std::cout << "Demo server: http://127.0.0.1:" << port << "/software-defined-platform\n";
    while (running) {
        const int client = accept(server, nullptr, nullptr);
        if (client >= 0) { serve_client(client, root); close(client); }
    }
    close(server);
    return 0;
}
