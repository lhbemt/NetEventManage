cc = g++
flags = -g -Wall -std=c++11 -pthread
objs = main.o threadpool.o clientmanage.o servermanage.o signalmanage.o timermanage.o clientsocket.o
target = test

$(target) : $(objs)
	$(cc) $(flags) -o $@ $^

main.o : main.cpp
	$(cc) $(flags) -c $^ -o $@

servermanage.o : ./source/CServerManage.cpp
	$(cc) $(flags) -c $^ -o $@ -I ./include

clientmanage.o : ./source/cclientmanage.cpp
	$(cc) $(flags) -c $^ -o $@ -I ./include

clientsocket.o : ./source/ctcpsocket.cpp
	$(cc) $(flags) -c $^ -o $@ -I ./include

signalmanage.o : ./source/csignalmanage.cpp
	$(cc) $(flags) -c $^ -o $@ -I ./include

timermanage.o : ./source/ctimermanage.cpp
	$(cc) $(flags) -c $^ -o $@ -I ./include

threadpool.o : ./source/CThreadPool.cpp
	$(cc) $(flags) -c $^ -o $@ -I ./include

clean:
	rm -rf $(target) $(objs)
