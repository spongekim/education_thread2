
#pragma once

#include <memory>
#include <thread>
#include <mutex>
#include <functional>
//#include <pthread.h>
class WorkerThread
{
public:
	WorkerThread() = default;
	virtual ~WorkerThread() = default;

	WorkerThread(const WorkerThread &reader) = delete;
	WorkerThread(WorkerThread &&reader) = delete;
	WorkerThread& operator =(const WorkerThread &reader) = delete;
	WorkerThread& operator =(WorkerThread &&reader) = delete;
	void setThreadBody(const std::string& name, std::function<void()> in_thread_body) {
		thread_name = name;
		thread_body = in_thread_body;
	}

	void start(void)
	{
		std::unique_lock<std::mutex> lock(is_running_mutex);
		if (is_running) {
			return;
		}

		read_thread = std::move(std::unique_ptr<std::thread>(
			new std::thread([this]() {
			//pthread_setname_np(pthread_self(), thread_name.c_str());
			if (thread_body) {
				thread_body();
			}
		})
			));
		is_running = true;
	}

	virtual void stop(void)
	{
		std::unique_lock<std::mutex> lock(is_running_mutex);
		if (!is_running) {
			return;
		}
		is_running = false;

		auto thread = std::move(read_thread);
		lock.unlock();

		if (thread->joinable()) {
			thread->join();
		}
	}

	bool isRunning(void) {
		std::unique_lock<std::mutex> lock(is_running_mutex);
		return is_running;
	}

protected:
	std::function<void()> thread_body = nullptr;
	bool is_running = false;
	std::string thread_name;

private:
	std::unique_ptr<std::thread> read_thread;
	std::mutex is_running_mutex;
};
