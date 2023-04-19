#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include <functional>
#include <atomic>
#include <thread>
#include <condition_variable>
#include <future>
#include "task_queue.h"

class ThreadPool {
public:
    ThreadPool(const int n_threads = 4)
        : m_threads(std::vector<std::thread>(n_threads)), is_shutdown(false) {}

    ThreadPool(const ThreadPool &) = delete;
    ThreadPool(ThreadPool &&) = delete;
    ThreadPool & operator=(const ThreadPool &) = delete;
    ThreadPool & operator=(ThreadPool &&) = delete;

    void init() {
        for (int i = 0; i < m_threads.size(); ++i) {
            // at做边界检查，越界抛出异常；传入的是一个伪函数可调用对象
            m_threads.at(i) = std::thread(ThreadWorker(i, this));
        } 
    }

    void shutdown() {
        is_shutdown = true;
        m_cv.notify_all();
        for(auto& i : m_threads) {
            if(i.joinable()) 
                i.join();
        }
    }

    // 可变参数模版
    template <typename F, typename... Args>
 #if __cplusplus > 201103L
  auto submit(F&& f, Args&& ...args) // c++14可行，c++11 需要添加尾返回类型推导  
#else
  auto submit(F&& f, Args&& ...args) -> std::future<decltype(f(args...))>   
#endif
    {   
        // 获取根据函数、输入参数推断返回结果
        //using Ret = typename std::result_of<F(Args...)>::type;
        using Ret = decltype(f(args...));

        std::function<Ret()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)... );

        // std::packaged_task包装以便可以异步调用
        auto task_ptr = std::make_shared<std::packaged_task<Ret()> >(func);

        std::function<void()> wrapper_func = [task_ptr](){
            (*task_ptr)();
        };

        // 外部线程执行
        m_queue.enqueue(wrapper_func);
        m_cv.notify_one();  // 唤醒一个线程， 异步执行当前任务
        
        // 返回工作的future指针
        return task_ptr->get_future();
    }

private:
    class ThreadWorker {
    public:
        ThreadWorker(const int id, ThreadPool* pool)
            : m_id(id), m_pool(pool) {}
        // 仿函数
        void operator()() {
            printf("threadpool [%d]: enter.\n", m_id);
            std::function<void()> func;
            bool hastask;
            while(!m_pool->is_shutdown) {
                {
                    std::unique_lock<std::mutex> lck(m_pool-> m_mutex);
                    printf("threadpool [%d]: waiting task.\n", m_id);
                    m_pool->m_cv.wait(lck, [&]{
                        // 在任务队列为空时，也能退出等待
                        if(m_pool->is_shutdown)
                            return true;
                        return !m_pool->m_queue.isEmpty();
                    });
                }
                hastask = m_pool->m_queue.dequeue(func);
                if(hastask) {
                    // 执行任务
                    printf("threadpool [%d]: excuting work... \n", m_id);
                    func();
                    printf("threadpool [%d]: work done. \n", m_id);
                }
            }
            printf("threadpool [%d]: exit.\n", m_id);
        }

    private:
        int m_id;
        ThreadPool* m_pool;
    };
private:
    TaskQueue<std::function<void()>> m_queue;
    std::atomic_bool is_shutdown;
    std::vector<std::thread> m_threads;
    std::mutex m_mutex;
    std::condition_variable m_cv;
};


#endif