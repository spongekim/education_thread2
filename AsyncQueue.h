#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <atomic>
#include <stdio.h>
 /**
  * @brief Thread-Safe asynchronous queue for getting
  * results from worker's threads.
  */
template<typename T>
class AsyncQueue
{
public:
	virtual void setCapacity(const unsigned int max_size)
	{
		m_capacity = max_size;
	}

	virtual bool push(T t)
	{
		std::lock_guard<std::mutex> lock(mutex);
		bool is_overflow = false;
		if (m_capacity > 0 && queue.size() >= m_capacity)
		{
			queue.pop();
			is_overflow = true;
		}
		queue.push(std::move(t));

		result_ready.notify_one();
		return is_overflow;
	}

	virtual T pop()
	{
		std::unique_lock<std::mutex> lock(mutex);
		result_ready.wait(lock, [this] { return !queue.empty(); });
		T ret = std::move(queue.front());
		queue.pop();
		return ret;
	}

	virtual T tryPop(const std::chrono::milliseconds& timeout = std::chrono::milliseconds(10))
	{
		std::unique_lock<std::mutex> lock(mutex);
		while (queue.empty()) {
			if (result_ready.wait_for(lock, timeout) == std::cv_status::timeout) {
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				return T{};
			}
		}
		T ret = std::move(queue.front());
		queue.pop();
		return ret;
	}

	int size()
	{
		std::unique_lock<std::mutex> lock(mutex);
		return queue.size();
	}
	bool empty()
	{
		std::unique_lock<std::mutex> lock(mutex);
		return queue.empty();
	}

	bool clear()
	{
		std::unique_lock<std::mutex> lock(mutex);
		std::queue<T> empty;
		std::swap(queue, empty);
		return true;
	}
	AsyncQueue() = default;
	virtual ~AsyncQueue() = default;
private:
	std::queue<T> queue;
	std::mutex mutex;
	std::condition_variable result_ready;
	unsigned int m_capacity{ 0u };
};

