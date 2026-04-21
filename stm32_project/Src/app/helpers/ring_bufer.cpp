#include "app/helpers/ring_buffer.hpp"

#include <cstring>

#define RING_BUFFER_IMPL(CAPACITY) \
RingBuffer##CAPACITY::RingBuffer##CAPACITY() \
    : m_head(0), m_tail(0) \
{ \
} \
\
RingBuffer##CAPACITY::~RingBuffer##CAPACITY() \
{ \
    /* No dynamic memory to free */ \
} \
\
bool RingBuffer##CAPACITY::Push(uint8_t value) \
{ \
    if (IsFull()) \
    { \
        return false; \
    } \
    m_buffer[m_head] = value; \
    m_head = (m_head + 1) % Capacity; \
    return true; \
} \
\
bool RingBuffer##CAPACITY::Pop(uint8_t& value) \
{ \
    if (IsEmpty()) \
    { \
        return false; \
    } \
    value = m_buffer[m_tail]; \
    m_tail = (m_tail + 1) % Capacity; \
    return true; \
} \
\
bool RingBuffer##CAPACITY::Pick(uint8_t& value) \
{ \
    if (IsEmpty()) \
    { \
        return false; \
    } \
    value = m_buffer[m_tail]; \
    return true; \
} \
\
bool RingBuffer##CAPACITY::IsEmpty() const \
{ \
    return m_head == m_tail; \
} \
\
bool RingBuffer##CAPACITY::IsFull() const \
{ \
    return ((m_head + 1) % Capacity) == m_tail; \
} \
\
uint8_t RingBuffer##CAPACITY::Size() const \
{ \
    if (m_head >= m_tail) \
    { \
        return m_head - m_tail; \
    } \
    else \
    { \
        return Capacity - (m_tail - m_head); \
    } \
}

RING_BUFFER_IMPL(4)
RING_BUFFER_IMPL(8)
RING_BUFFER_IMPL(16)
RING_BUFFER_IMPL(32)
RING_BUFFER_IMPL(64)
RING_BUFFER_IMPL(128)
RING_BUFFER_IMPL(256)