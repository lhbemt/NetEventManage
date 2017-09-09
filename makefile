cc = g++
flags = -g -Wall -std=c++11 -pthread
objs = main.o threadpool.o
target = test

$(target) : $(objs)
	$(cc) $(flags) -o $@ $^

main.o : main.cpp
	$(cc) $(flags) -c $^ -o $@

threadpool.o : ./source/CThreadPool.cpp
	$(cc) $(flags) -c $^ -o $@ -I ./include

clean:
	rm -rf $(target) $(objs)
