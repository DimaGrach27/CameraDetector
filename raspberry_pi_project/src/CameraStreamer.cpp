#include "CameraStreamer.h"

#include <iostream>
#include <string>
#include <chrono>
#include <thread>

#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/logger.hpp>

#include "FaceTracker.h"
#include "Connection/RPiPacket.h"
#include "Connection/IConnection.h"

CameraStreamer::CameraStreamer(IConnection& connection)
    :m_connection(connection)
{

}

CameraStreamer::~CameraStreamer()
{

}

bool CameraStreamer::Start()
{
    const std::string pipeline =
        "libcamerasrc camera-name=/base/axi/pcie@1000120000/rp1/i2c@88000/imx219@10 ! "
        "video/x-raw,format=RGBx,width=640,height=480,framerate=30/1 ! "
        "videoconvert ! "
        "video/x-raw,format=BGR ! "
        "appsink drop=true max-buffers=1 sync=false";

    // OpenCV asks live GStreamer sources for duration/position during open().
    // libcamerasrc does not provide those values, so temporarily hide that noise.
    const auto previousLogLevel = cv::utils::logging::getLogLevel();
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_ERROR);
    const bool openedWithGstreamer = m_camera.open(pipeline, cv::CAP_GSTREAMER);
    cv::utils::logging::setLogLevel(previousLogLevel);

    if (openedWithGstreamer && m_camera.isOpened())
    {
        std::cout << "Camera opened with GStreamer pipeline." << std::endl;
        return true;
    }

    std::cout << "Failed to open camera with GStreamer pipeline, trying /dev/video0." << std::endl;

    if (m_camera.open(0))
    {
        std::cout << "Camera opened via default backend." << std::endl;
        return true;
    }

    std::cout << "Cannot open camera." << std::endl;
    return false;
}

void CameraStreamer::Stop()
{
    m_camera.release();
}

void CameraStreamer::Update(int frameCount)
{
    constexpr int JpegQuality = 70;

    std::vector<unsigned char> jpegBuffer;
    std::vector<int> encodeParams =
    {
        cv::IMWRITE_JPEG_QUALITY,
        JpegQuality
    };

    if (!m_camera.read(m_frame))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        return;
    }

    if (m_frame.empty())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        return;
    }

    cv::flip(m_frame, m_frame, 1);

    m_faceTracker.StartTrack(m_frame, frameCount);
    m_faceTracker.Track(m_frame);

    if (!cv::imencode(".jpg", m_frame, jpegBuffer, encodeParams))
    {
        return;
    }

    m_frameBuffer.Update(jpegBuffer);

    constexpr float DeathZoneThreshold = 30.0f;
    constexpr float MoveSpeed = 10.0f;
    uint8_t moveValue = static_cast<uint8_t>(std::abs(m_faceTracker.GetHorizontalOffsetNormalized()) * MoveSpeed);
    if (m_faceTracker.GetHorizontalOffsetPx() > DeathZoneThreshold)
    {
        m_connection.SendPacket(PacketStruct{0, MessageTypes::Command, CommandTypes::MotorRight, moveValue});
        m_currentDetection = false;
    }
    else if ((m_faceTracker.GetHorizontalOffsetPx() < -DeathZoneThreshold))
    {
        m_connection.SendPacket(PacketStruct{0, MessageTypes::Command, CommandTypes::MotorLeft, moveValue});
        m_currentDetection = false;
    }
    else
    {
        m_currentDetection = true;
    }

    if (m_previousDetection != m_currentDetection)
    {
        if (m_currentDetection)
        {
            m_connection.SendPacket(PacketStruct{0, MessageTypes::Command, CommandTypes::SetLED, 1});
        }
        else
        {
            m_connection.SendPacket(PacketStruct{0, MessageTypes::Command, CommandTypes::SetLED, 0});
        }
    }

    m_previousDetection = m_currentDetection;

    constexpr int FrameSleepMs = 1;
    std::this_thread::sleep_for(std::chrono::milliseconds(FrameSleepMs));
}

bool CameraStreamer::IsOpened() const
{
    return m_camera.isOpened();
}

bool CameraStreamer::GetLatestJpeg(std::vector<unsigned char>& outData) const
{
    return m_frameBuffer.GetCopy(outData);
}
