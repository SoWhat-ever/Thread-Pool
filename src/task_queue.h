#ifndef __TASK_QUEUE_H__
#define __TASK_QUEUE_H__

#include <queue>
#include <mutex>

template<typename T>
class TaskQueue {
public:
    TaskQueue() = default;
    TaskQueue(TaskQueue&&) = default;
    TaskQueue& operator=(TaskQueue&&) = default;

    TaskQueue(const TaskQueue&) = delete;
    TaskQueue& operator=(const TaskQueue&) = delete;

    ~TaskQueue() { }

    void enqueue(const T& t) {
        std::unique_lock<std::mutex> lck(m_mutex);
        m_queue.push(t);
    }
    
    bool dequeue(T& t) {
        std::unique_lock<std::mutex> lck(m_mutex);
        if(m_queue.empty()) {
            return false;
        }
        // 转换为右值，减少不必要的拷贝
        t = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }

    void clean() {
        std::unique_lock<std::mutex> lck(m_mutex);
        while(!m_queue.empty())
            m_queue.pop();
    }

    int getTaskSize() {
        std::unique_lock<std::mutex> lck(m_mutex);
        return m_queue.size();
    }

    bool isEmpty() {
        std::unique_lock<std::mutex> lck(m_mutex);
        return m_queue.empty();
    }
private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
};

#endif