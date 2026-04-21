#pragma once

#include <cstdint>
#include <cstddef>

#define DECLARE_RING_BUFFER(CAPACITY) \
class RingBuffer##CAPACITY \
{ \
public: \
    RingBuffer##CAPACITY(); \
    ~RingBuffer##CAPACITY(); \
 \
    bool Push(uint8_t value); \
    bool Pop(uint8_t& value); \
    bool Pick(uint8_t& value); \
    bool IsEmpty() const; \
    bool IsFull() const; \
    uint8_t Size() const; \
private: \
    static constexpr size_t Capacity = CAPACITY; \
    uint8_t m_buffer[Capacity]; \
    volatile uint8_t m_head; \
    volatile uint8_t m_tail; \
}; 


DECLARE_RING_BUFFER(4)
DECLARE_RING_BUFFER(8)
DECLARE_RING_BUFFER(16)
DECLARE_RING_BUFFER(32)
DECLARE_RING_BUFFER(64)
DECLARE_RING_BUFFER(128)
DECLARE_RING_BUFFER(256)