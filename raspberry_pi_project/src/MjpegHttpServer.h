#pragma once

#include "CameraStreamer.h"

#include <atomic>
#include <thread>

class MjpegHttpServer
{
public:
    explicit MjpegHttpServer(CameraStreamer& streamer);
    ~MjpegHttpServer();

    bool Start();
    void Stop();

private:
    void AcceptLoop();

    void HandleClient(int clientSocket);

    void SendIndexPage(int clientSocket);
    void SendNotFound(int clientSocket);
    void SendStream(int clientSocket);

private:
    CameraStreamer& m_streamer;
    std::atomic<bool> m_isRunning;
    int m_serverSocket;
    std::thread m_acceptThread;
};
