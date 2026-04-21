#pragma once

#include <vector>
#include <mutex>

class FrameBuffer
{
public:
    void Update(const std::vector<unsigned char>& jpegData);
    bool GetCopy(std::vector<unsigned char>& outData) const;

private:
    mutable std::mutex m_mutex;
    std::vector<unsigned char> m_jpegData;
};