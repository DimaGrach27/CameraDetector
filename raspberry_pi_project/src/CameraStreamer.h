#pragma once

#include <vector>

#include <opencv2/opencv.hpp>

#include "FaceTracker.h"
#include "FrameBuffer.h"
#include "Connection/IConnection.h"

class CameraStreamer
{
public:
    CameraStreamer(IConnection& connection);
    ~CameraStreamer();

    bool Start();
    void Stop();
    void Update(int frameCount);

    bool IsOpened() const;
    bool GetLatestJpeg(std::vector<unsigned char>& outData) const;

private:
    cv::VideoCapture m_camera;
    cv::Mat m_frame;

    FaceTracker m_faceTracker;
    FrameBuffer m_frameBuffer;

    IConnection& m_connection;

    bool m_previousDetection = false;
    bool m_currentDetection = false;
};
