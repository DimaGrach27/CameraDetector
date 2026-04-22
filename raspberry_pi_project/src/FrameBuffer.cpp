#include "FrameBuffer.h"

#include <vector>
#include <mutex>

void FrameBuffer::Update(const std::vector<unsigned char>& jpegData)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_jpegData = jpegData;
}

bool FrameBuffer::GetCopy(std::vector<unsigned char>& outData) const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_jpegData.empty())
    {
        return false;
    }

    outData = m_jpegData;
    return true;
}