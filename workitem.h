#pragma once

#include <string>
#include <algorithm>
#include <map>
#include <mutex>
#include <string>
#include "progressbar.h"

extern std::map<int, std::string> processed;
extern std::unique_ptr<progressbar> bar;
extern std::mutex bar_mutex;
class WorkItem
{
public:
	WorkItem() = delete;
	virtual ~WorkItem() = default;

	WorkItem(int idx_, std::string str_){
		idx = idx_;
		str = str_;
	}
	int get_idx(void) { return idx; }
	
	std::string get_str(void) {	return str;}

	void process(void) {
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			{
				std::unique_lock<std::mutex> lock(bar_mutex);
				std::sort(str.begin(), str.end());
				processed[idx] = str;
				bar->update();
			}
		}
	}
protected: 
	int idx = -1;
	std::string str;
};
