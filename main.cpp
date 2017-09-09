#include "include/CThreadPool.h"
#include <unistd.h>

using namespace std;

void funcHello(void* arg)
{
    std::cout << "hello world" << std::endl;
}

int main()
{
    // test threadpool
    CThreadPool pool(4);
    bool bRet = pool.Start();
    if (!bRet)
        std::cout << "start threadpool failed" << std::endl;
    for (int i = 0; i < 10; ++i)
    {
        Task task;
        task.func = funcHello;
        task.arg = nullptr;
        pool.AddTask(std::move(task));
    }
    sleep(10);
    pool.Stop();
    return 0;
}
