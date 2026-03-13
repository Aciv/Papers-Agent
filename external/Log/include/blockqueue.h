#pragma once

#include <mutex>
#include <deque>
#include <condition_variable>
#include <sys/time.h>
#include <cassert>

template<class T>
class BlockDeque {
public:
    explicit BlockDeque(std::size_t _MaxCapacity = 1000);

    ~BlockDeque();

    void clear();
    bool empty();
    bool full();
    void Close();

    std::size_t size();
    std::size_t capacity();

    T front();
    T back();

    void push_back(const T &&_item);
    void push_front(const T &&_item);

    bool pop(T &_item);
    bool pop(T &_item, int timeout);

    void flush();

private:
    std::deque<T> m_deq;
    std::size_t m_capacity;
    bool m_Closed;

    std::mutex m_mtx;
    std::condition_variable m_condConsumer;
    std::condition_variable m_condProducer;
};


template<class T>
BlockDeque<T>::BlockDeque(std::size_t _MaxCapacity) :m_capacity(_MaxCapacity) {
    assert(_MaxCapacity > 0);
    m_Closed = false;
}

template<class T>
BlockDeque<T>::~BlockDeque() {
    Close();
};

template<class T>
void BlockDeque<T>::Close() {
    {   
        
        std::lock_guard<std::mutex> locker(m_mtx);
        m_deq.clear();
        m_Closed = true;
    }
    m_condProducer.notify_all();
    m_condConsumer.notify_all();
};

template<class T>
void BlockDeque<T>::flush() {
    m_condConsumer.notify_one();
};

template<class T>
void BlockDeque<T>::clear() {
    std::lock_guard<std::mutex> locker(m_mtx);
    m_deq.clear();
}

template<class T>
T BlockDeque<T>::front() {
    std::lock_guard<std::mutex> locker(m_mtx);
    return m_deq.front();
}

template<class T>
T BlockDeque<T>::back() {
    std::lock_guard<std::mutex> locker(m_mtx);
    return m_deq.back();
}

template<class T>
std::size_t BlockDeque<T>::size() {
    std::lock_guard<std::mutex> locker(m_mtx);
    return m_deq.size();
}

template<class T>
std::size_t BlockDeque<T>::capacity() {
    std::lock_guard<std::mutex> locker(m_mtx);
    return m_capacity;
}


template<class T>
void BlockDeque<T>::push_back(const T &&_item) {
    // unique_lock allows lock and unlock manually
    // and condition_variable wait must unlock the mutex
    std::unique_lock<std::mutex> locker(m_mtx);
    m_condProducer.wait(locker, [this]() 
            { return m_deq.size() < m_capacity; });
    m_deq.emplace_back(_item);
    m_condConsumer.notify_one();
}

template<class T>
void BlockDeque<T>::push_front(const T &&_item) {
    std::unique_lock<std::mutex> locker(m_mtx);

    m_condProducer.wait(locker, [this]() 
            { return m_deq.size() < m_capacity; });
    
    m_deq.emplace_front(_item);
    m_condConsumer.notify_one();
}

template<class T>
bool BlockDeque<T>::empty() {
    std::lock_guard<std::mutex> locker(m_mtx);
    return m_deq.empty();
}

template<class T>
bool BlockDeque<T>::full(){
    std::lock_guard<std::mutex> locker(m_mtx);
    return m_deq.size() >= m_capacity;
}

template<class T>
bool BlockDeque<T>::pop(T &_item) {
    std::unique_lock<std::mutex> locker(m_mtx);

    m_condConsumer.wait(locker, [this]()
            { return !m_deq.empty() || m_Closed; } );

    if(m_Closed && m_deq.empty()){
        return false;
    }
    
    _item = std::move(m_deq.front());
    m_deq.pop_front();
    m_condProducer.notify_one();
    return true;
}

template<class T>
bool BlockDeque<T>::pop(T &_item, int _timeout) {
    std::unique_lock<std::mutex> locker(m_mtx);
    while(m_deq.empty()){
        if(m_condConsumer.wait_for(locker, std::chrono::seconds(_timeout)) 
                == std::cv_status::timeout){
            return false;
        }
        if(m_Closed){
            return false;
        }
    }
    _item = std::move(m_deq.front());
    m_deq.pop_front();
    m_condProducer.notify_one();
    return true;
}

