#include "./include/cservermanage.h"
#include <unistd.h>
#include <signal.h>

using namespace std;

void funcHello(void* arg)
{
    std::cout << "hello world" << std::endl;
}

int main()
{
    // test threadpool
//    CThreadPool pool(4);
//    bool bRet = pool.Start();
//    if (!bRet)
//        std::cout << "start threadpool failed" << std::endl;
//    sleep(5);
//    for (int i = 0; i < 10; ++i)
//    {
//        Task task;
//        task.func = funcHello;
//        task.arg = nullptr;
//        pool.AddTask(std::move(task));
//    }
//    sleep(10);
//    pool.Stop();
//    return 0;

	// test servermanage
	CServerManage manage;
	bool bRet = manage.Start();
	if (!bRet)
	{
	  std::cout << "server start error" << std::endl;
	  return false;
	}
	
//	struct EventBase env;
//	env.fd = 1; //timer id
//	env.callBack = funcHello;
//	env.arg = nullptr;
//  manage.RegisterEvent(EVENT_TYPE::EVENT_TIMER, env, 1000); // 1s
//  sleep(5);
//  manage.UnRegisterEvent(EVENT_TYPE::EVENT_TIMER, env);
//  sleep(2);
//
	// test socket echo
//	while(1)
//	{}
	
	// test signal
	struct EventBase env;
	env.fd = SIGINT; //signal
	env.callBack = funcHello;
	env.arg = nullptr;
	manage.RegisterEvent(EVENT_TYPE::EVENT_SIG, env);
	sleep(5);
	manage.UnRegisterEvent(EVENT_TYPE::EVENT_SIG, env);
	manage.Stop();
	return 0;
}
