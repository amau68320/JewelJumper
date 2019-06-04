#pragma once
#include <mgpcl/Mem.h>
#include <mgpcl/Assert.h>

/*
 * Classe de tourniquet. Surtout utilise pour des graphiques.
 */
template<typename T> class RoundRobin
{
public:
    RoundRobin() : m_data(nullptr), m_size(0), m_pos(0)
    {
    }

    RoundRobin(int sz) : m_size(sz), m_pos(0)
    {
        m_data = m::mem::alloc<T>(static_cast<size_t>(sz));

        for(int i = 0; i < sz; i++)
            new(m_data + i) T();
    }

    RoundRobin(const RoundRobin<T> &src) : m_size(src.m_size), m_pos(src.m_pos)
    {
        if(m_data == nullptr)
            m_data = nullptr;
        else {
            m_data = m::mem::alloc<T>(static_cast<size_t>(m_size));
            m::mem::copyInitT<T>(m_data, src.m_data, static_cast<size_t>(m_size));
        }
    }

    RoundRobin(RoundRobin<T> &&src) noexcept : m_data(src.m_data), m_size(src.m_size), m_pos(src.m_pos)
    {
        src.m_data = nullptr;
        src.m_size = 0;
    }

    ~RoundRobin()
    {
        if(m_data != nullptr) {
            for(int i = 0; i < m_size; i++)
                m_data[i].~T();

            m::mem::del<T>(m_data);
        }
    }

    void push(const T &data)
    {
        m_data[m_pos] = data;
        m_pos = (m_pos + 1) % m_size;
    }

    void push(T &&data)
    {
        m_data[m_pos] = std::move(data);
        m_pos = (m_pos + 1) % m_size;
    }

    int size() const
    {
        return m_size;
    }

    int operator ~ () const
    {
        return m_size;
    }

    T &operator[] (int idx)
    {
        mDebugAssert(idx >= 0 && idx < m_size, "invalid range");
        return m_data[(m_pos + idx) % m_size];
    }

    const T &operator[] (int idx) const
    {
        mDebugAssert(idx >= 0 && idx < m_size, "invalid range");
        return m_data[(m_pos + idx) % m_size];
    }

    RoundRobin<T> &operator = (const RoundRobin<T> &src)
    {
        if(m_data == src.m_data)
            return *this;

        if(m_data != nullptr) {
            for(int i = 0; i < m_size; i++)
                m_data[i].~T();
        }

        if(m_size != src.m_size) {
            m_size = src.m_size;
            m::mem::del<T>(m_data);

            if(src.m_data == nullptr)
                m_data = nullptr;
            else
                m_data = m::mem::alloc<T>(static_cast<size_t>(m_size));
        }

        if(src.m_data != nullptr)
            m::mem::copyInitT<T>(m_data, src.m_data, static_cast<size_t>(m_size));

        m_pos = src.m_pos;
        return *this;
    }

    RoundRobin<T> &operator = (RoundRobin<T> &&src) noexcept
    {
        m_data = src.m_data;
        m_size = src.m_size;
        m_pos = src.m_pos;

        src.m_data = nullptr;
        src.m_size = 0;

        return *this;
    }

private:
    T *m_data;
    int m_size;
    int m_pos;
};
