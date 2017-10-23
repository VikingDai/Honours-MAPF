#pragma once

// pqueue.h
//
// A min priority queue. Based on an implementation from warthog
// by Daniel Harabor (https://bitbucket.org/dharabor/pathfinding/src/90254385c6cdc3b0d208adea3d0d3bc1c7dbc5c6/src/util/pqueue.h)

#include <iostream>

class SearchNode
{
	int GetPriority() { return 0; }
};

template <class T>
class PriorityQueue
{
public:
	PriorityQueue<T>(int maxSize, bool minQueue)
		: maxSize(maxSize), minQueue(minQueue), queueSize(0), arr(nullptr)
	{
		Resize(maxSize);
	}

	~PriorityQueue()
	{
		delete[] arr;
	}

	void Clear()
	{
		for (unsigned int i = 0; i < queueSize; i++)
		{
			arr[i] = 0;
		}
		queueSize = 0;
	}

	void DecreaseKey(T val)
	{
		assert(val->GetPriority() < queueSize);
		if (minQueue)
		{
			HeapifyUp(val->GetPriority());
		}
		else
		{
			HeapifyDown(val->GetPriority());
		}
	}

	void IncreaseKey(T val)
	{
		assert(val->GetPriority() < queueSize);
		if (minQueue)
		{
			HeapifyDown(val->GetPriority());
		}
		else
		{
			HeapifyUp(val->GetPriority());
		}
	}

	void Push(T val)
	{
		if (Contains(val))
			return;

		if (queueSize + 1 > maxSize)
			Resize(maxSize * 2);

		int priority = queueSize;
		arr[priority] = val;

		val->SetPriority(priority);
		queueSize += 1;
		HeapifyUp(priority);
	}

	T Pop()
	{
		if (queueSize == 0) return nullptr;

		T ans = arr[0];
		queueSize -= 1;

		if (queueSize > 0)
		{
			arr[0] = arr[queueSize];
			arr[0]->SetPriority(0);
			HeapifyDown(0);
		}

		return ans;
	}

	bool Empty()
	{
		return queueSize == 0;
	}

	bool Contains(T val)
	{
		int priority = val->GetPriority();
		if (priority < queueSize && &val == &arr[priority])
		{
			return true;
		}
		return false;
	}

	T Peek()
	{
		if (queueSize > 0)
			return arr[0];

		return nullptr;
	}

	int Size()
	{
		return queueSize;
	}

	bool IsMinQueue()
	{
		return minQueue;
	}

	int Mem()
	{
		return maxSize * sizeof(T) + sizeof(*this);
	}

//private:
	int maxSize;
	bool minQueue;
	int queueSize;
	T* arr;

	void HeapifyUp(int index)
	{
		assert(index < queueSize);
		while (index > 0)
		{
			int parent = (index - 1) >> 1;
			if (Rotate(arr[parent], arr[index]))
			{
				Swap(parent, index);
				index = parent;
			}
			else
			{
				break;
			}
		}
	}

	void HeapifyDown(int index)
	{
		int firstLeafIndex = queueSize >> 1;
		while (index < firstLeafIndex)
		{
			/** Find smallest (or largest, depending on heap type) child */
			int child1 = (index << 1) + 1;
			int child2 = (index << 1) + 2;
			int which = child1;

			if ((child2 < queueSize) && *arr[child2] < *arr[child1])
			{
				which = child2;
			}

			if (*arr[which] < *arr[index])
			{
				Swap(index, which);
				index = which;
			}
			else
			{
				break;
			}
		}
	}

	void Resize(int newSize)
	{
		//std::cout << "pqueue::resize oldsize: " << queueSize << " newsize " << newSize << std::endl;
		if (newSize < queueSize)
		{
			std::cerr << "err; pqueue::resize newsize < queuesize " << std::endl;
			exit(1);
		}

		T* tmp = new T[newSize];
		for (unsigned int i = 0; i < queueSize; i++)
		{
			tmp[i] = arr[i];
		}
		delete[] arr;
		arr = tmp;
		maxSize = newSize;
	}

	bool Rotate(T first, T second)
	{
		if (minQueue)
		{
			if (*second < *first) return true;
		}
		else
		{
			if (*second > *first) return true;
		}

		return false;
	}

	void Swap(int index1, int index2)
	{
		assert(index1 < queueSize && index2 < queueSize);

		T tmp = arr[index1];
		arr[index1] = arr[index2];

		arr[index1]->SetPriority(index1);
		arr[index2] = tmp;
		tmp->SetPriority(index2);
	}

	friend std::ostream& operator<<(std::ostream& os, PriorityQueue& queue)
	{
		os << "Priority Queue: " << std::endl;
		for (int i = 0; i < queue.queueSize; i++)
			os << "\t" << *queue.arr[i] << std::endl;
		return os;
	}

};
