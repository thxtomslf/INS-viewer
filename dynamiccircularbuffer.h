#ifndef DYNAMICCIRCULARBUFFER_H
#define DYNAMICCIRCULARBUFFER_H

#include <QByteArray>
#include <QDebug>
#include <stdexcept>

class DynamicCircularBuffer {
public:
    explicit DynamicCircularBuffer(int initialCapacity = 4)
        : m_capacity(initialCapacity), m_size(0), m_start(0), m_buffer(initialCapacity, '\0') {}

    void append(const QByteArray& data) {
        int dataSize = data.size();
        int availableSpace = m_capacity - m_size;

        // Reallocate if necessary
        if (dataSize > availableSpace) {
            reallocateBuffer(m_size + dataSize);
        }

        // Insert new data
        for (int i = 0; i < dataSize; ++i) {
            m_buffer[(m_start + m_size) % m_capacity] = data[i];
            ++m_size;
        }
    }

    QByteArray read(int bytes) const {
        if (bytes > m_size) {
            throw std::out_of_range("Requested more bytes than available in buffer.");
        }

        QByteArray result;
        result.reserve(bytes);
        for (int i = 0; i < bytes; ++i) {
            result.append(m_buffer[(m_start + i) % m_capacity]);
        }
        return result;
    }

    QByteArray pop(int bytes) {
        QByteArray result = read(bytes);
        m_start = (m_start + bytes) % m_capacity;
        m_size -= bytes;
        return result;
    }

    QByteArray toByteArray() const {
        return read(m_size);
    }

    int size() const {
        return m_size;
    }

    bool isEmpty() const {
        return m_size == 0;
    }

private:
    void reallocateBuffer(int newCapacity) {
        QByteArray newBuffer(newCapacity, '\0');

        for (int i = 0; i < m_size; ++i) {
            newBuffer[i] = m_buffer[(m_start + i) % m_capacity];
        }

        m_buffer = std::move(newBuffer);
        m_capacity = newCapacity;
        m_start = 0;
    }

    QByteArray m_buffer;
    int m_capacity;
    int m_size;
    int m_start;
};

#endif // DYNAMICCIRCULARBUFFER_H
