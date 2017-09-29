#ifndef __CMEMORYPOOL__H
#define __CMEMORYPOOL__H

// 内存池模块
#include <list>
#include <vector>
#include <iostream>
#include <mutex>
#include <algorithm>
#include <thread>
template<class T>
struct TBuff
{
	T* pBuff; // 指针
	int nCount;
	TBuff()
	{
		pBuff = nullptr;
		nCount = 0;
	}
};

template<class T>
class CMemoryPool
{
public:
	CMemoryPool();
	~CMemoryPool();

public:
	T* GetElement(); // 获取一个指针 失败返回nullptr
	void ReleaseElement(T*); // 放回一个指针

private:
	std::list<T*> m_lstFree; // 空闲的内存块链表
	std::list<T*> m_lstUse; // 正在使用的内存块链表
	unsigned int  m_nElementCount;
	std::vector<TBuff<T>> m_vectBuff; // 所有分配的内存 用于析构时释放

	std::mutex m_mutexFree; // 空闲锁 保证线程安全
	std::mutex m_mutexUse; // 使用锁 保证线程安全

};

template<class T>
CMemoryPool<T>::CMemoryPool()
{
	m_nElementCount = 1; // 初始化为1 
	TBuff<T> obj;
	obj.nCount = 1;
	obj.pBuff = new (std::nothrow) T[obj.nCount]; // 分配失败不抛出异常返回空指针
	m_vectBuff.push_back(obj);
	// 将分配的内存加入空闲队列
	m_lstFree.push_back(obj.pBuff);
}

template<class T>
CMemoryPool<T>::~CMemoryPool()
{
	while (1) // 等待所有的内存全被使用完
	{
		m_mutexUse.lock();
		if (m_lstUse.empty())
		{
			for (auto & obj : m_vectBuff)
				delete[] obj.pBuff;
			m_mutexUse.unlock();
			break;
		}
		else
		{
			m_mutexUse.unlock();
			std::this_thread::sleep_for(std::chrono::milliseconds(20)); // 睡眠一段时间再检测
		}
	}
}

template<class T>
T* CMemoryPool<T>::GetElement()
{
	m_mutexFree.lock();
	if (m_lstFree.empty()) // 没有空闲的内存
	{
		TBuff<T> obj;
		obj.nCount = 2 * m_nElementCount; // 上一次分配的两倍
		obj.pBuff = new (std::nothrow)T[obj.nCount];
		for (int i = 0; i < obj.nCount; ++i)
			m_lstFree.push_back(obj.pBuff + i);
		m_vectBuff.push_back(obj);
	}
	T* pHead = m_lstFree.front();
	m_lstFree.pop_front();
	m_mutexFree.unlock();
	m_mutexUse.lock();
	m_lstUse.push_back(pHead);
	m_mutexUse.unlock();
	return pHead;
}

template<class T>
void CMemoryPool<T>::ReleaseElement(T* pData)
{
	m_mutexUse.lock();
	auto iter = std::find_if(m_lstUse.begin(), m_lstUse.end(), [pData](T* pFind) { return pData == pFind; }); // 找到该指针
	if (iter != m_lstUse.end())
		m_lstUse.erase(iter);
	m_mutexUse.unlock();
	m_mutexFree.lock();
	m_lstFree.push_back(pData);
	m_mutexFree.unlock();
}

#endif
