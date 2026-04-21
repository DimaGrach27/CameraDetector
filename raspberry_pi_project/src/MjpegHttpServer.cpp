#include "MjpegHttpServer.h"

#include "CameraStreamer.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstring>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace
{
bool SendAll(int socket, const char* data, size_t size)
{
    size_t totalSent = 0;

    while (totalSent < size)
    {
        const ssize_t sent = send(socket, data + totalSent, size - totalSent, MSG_NOSIGNAL);
        if (sent <= 0)
        {
            return false;
        }

        totalSent += static_cast<size_t>(sent);
    }

    return true;
}

bool SendAll(int socket, const std::string& data)
{
    return SendAll(socket, data.c_str(), data.size());
}

std::string GetRequestPath(const std::string& request)
{
    const size_t lineEnd = request.find("\r\n");
    const std::string requestLine = request.substr(0, lineEnd);

    const size_t methodEnd = requestLine.find(' ');
    if (methodEnd == std::string::npos)
    {
        return {};
    }

    const size_t pathEnd = requestLine.find(' ', methodEnd + 1);
    if (pathEnd == std::string::npos)
    {
        return {};
    }

    return requestLine.substr(methodEnd + 1, pathEnd - methodEnd - 1);
}
}

MjpegHttpServer::MjpegHttpServer(CameraStreamer& streamer)
    : m_streamer(streamer),
      m_isRunning(false),
      m_serverSocket(-1)
{
}

MjpegHttpServer::~MjpegHttpServer()
{
    Stop();
}

bool MjpegHttpServer::Start()
{
    constexpr int ServerPort = 8080;
    constexpr int ClientBacklog = 10;

    std::signal(SIGPIPE, SIG_IGN);

    m_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_serverSocket < 0)
    {
        std::cerr << "Failed to create socket.\n";
        return false;
    }

    int reuse = 1;
    setsockopt(m_serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    sockaddr_in serverAddress {};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(ServerPort);

    if (bind(m_serverSocket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) < 0)
    {
        std::cerr << "Failed to bind socket.\n";
        close(m_serverSocket);
        m_serverSocket = -1;
        return false;
    }

    if (listen(m_serverSocket, ClientBacklog) < 0)
    {
        std::cerr << "Failed to listen on socket.\n";
        close(m_serverSocket);
        m_serverSocket = -1;
        return false;
    }

    m_isRunning = true;
    m_acceptThread = std::thread(&MjpegHttpServer::AcceptLoop, this);

    std::cout << "HTTP server started on port " << ServerPort << std::endl;
    return true;
}

void MjpegHttpServer::Stop()
{
    m_isRunning = false;

    if (m_serverSocket >= 0)
    {
        shutdown(m_serverSocket, SHUT_RDWR);
        close(m_serverSocket);
        m_serverSocket = -1;
    }

    if (m_acceptThread.joinable())
    {
        m_acceptThread.join();
    }
}

void MjpegHttpServer::AcceptLoop()
{
    while (m_isRunning)
    {
        sockaddr_in clientAddress {};
        socklen_t clientLength = sizeof(clientAddress);

        const int clientSocket = accept(m_serverSocket, reinterpret_cast<sockaddr*>(&clientAddress), &clientLength);

        if (clientSocket < 0)
        {
            if (m_isRunning)
            {
                constexpr int AcceptTimeoutMs = 200;

                std::this_thread::sleep_for(std::chrono::milliseconds(AcceptTimeoutMs));
            }

            continue;
        }

        std::thread(&MjpegHttpServer::HandleClient, this, clientSocket).detach();
    }
}

void MjpegHttpServer::HandleClient(int clientSocket)
{
    char requestBuffer[2048];
    std::memset(requestBuffer, 0, sizeof(requestBuffer));

    const ssize_t received = recv(clientSocket, requestBuffer, sizeof(requestBuffer) - 1, 0);
    if (received <= 0)
    {
        close(clientSocket);
        return;
    }

    const std::string request(requestBuffer);

    const std::string path = GetRequestPath(request);

    if (path == "/" || path.empty())
    {
        SendIndexPage(clientSocket);
        close(clientSocket);
        return;
    }

    if (path == "/stream" || path == "/stream.mjpg")
    {
        SendStream(clientSocket);
        close(clientSocket);
        return;
    }

    SendNotFound(clientSocket);
    close(clientSocket);
}

void MjpegHttpServer::SendIndexPage(int clientSocket)
{
    const std::string body =
        "<html>"
        "<head><title>Raspberry Pi Camera</title></head>"
        "<body>"
        "<h1>Raspberry Pi Camera Stream</h1>"
        "<img src=\"/stream.mjpg\" width=\"640\" />"
        "</body>"
        "</html>";

    const std::string response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Content-Length: " + std::to_string(body.size()) + "\r\n"
        "Connection: close\r\n"
        "\r\n" +
        body;

    SendAll(clientSocket, response);
}

void MjpegHttpServer::SendNotFound(int clientSocket)
{
    const std::string body = "404 Not Found";

    const std::string response =
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/plain; charset=utf-8\r\n"
        "Content-Length: " + std::to_string(body.size()) + "\r\n"
        "Connection: close\r\n"
        "\r\n" +
        body;

    SendAll(clientSocket, response);
}

void MjpegHttpServer::SendStream(int clientSocket)
{
    const std::string header =
        "HTTP/1.1 200 OK\r\n"
        "Cache-Control: no-cache\r\n"
        "Pragma: no-cache\r\n"
        "Connection: close\r\n"
        "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n"
        "\r\n";

    if (!SendAll(clientSocket, header))
    {
        return;
    }

    std::vector<unsigned char> jpegData;

    while (m_isRunning)
    {
        if (!m_streamer.GetLatestJpeg(jpegData))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        const std::string frameHeader =
            "--frame\r\n"
            "Content-Type: image/jpeg\r\n"
            "Content-Length: " + std::to_string(jpegData.size()) + "\r\n"
            "\r\n";

        if (!SendAll(clientSocket, frameHeader))
        {
            break;
        }

        if (!SendAll(clientSocket, reinterpret_cast<const char*>(jpegData.data()), jpegData.size()))
        {
            break;
        }

        if (!SendAll(clientSocket, "\r\n", 2))
        {
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(15));
    }
}
