#include "../src/task_queue.h"
#include "../src/thread_pool.h"

int main() {
    ThreadPool pools(4);
    pools.init();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    printf("===============================================\n");

    std::vector<std::future<int>> futs;

    for (int i = 0; i < 20; i++)
    {
        printf("submit task %d.\n", i);
        futs.push_back(pools.submit([i]{
            std::this_thread::sleep_for(std::chrono::microseconds( rand()%500 + 100));
            printf("task %d done!\n", i);
            return i;
        }));
    }

    printf("===============================================\n");
    // 等待10s。获取所有任务结果
    std::this_thread::sleep_for(std::chrono::seconds(5));
    for(auto& fut : futs){
        printf("result %d \n", fut.get());
    }

    printf("===============================================\n");
    pools.shutdown();
}