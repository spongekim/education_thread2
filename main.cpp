#include <stdio.h>
#include "workitem.h"
#include "AsyncQueue.h"
#include "workerthread.h"
#include <chrono>
#include <atomic>
#include <string>
#include <vector>
#include <map>
#include <random>
#include <algorithm>
#include "progressbar.h"

constexpr int kMaxQueueCapa = 10;
constexpr int kMaxProduce = 1000;

extern void user_init(int max_produce_);
extern void user_consume(std::unique_ptr<WorkItem> workitem);
extern void user_deinit(void);

std::map<int, std::string > processed;
std::string generate_random_string();
std::unique_ptr<progressbar> bar = std::make_unique<progressbar>(kMaxProduce);//https://github.com/gipert/progressbar
std::mutex bar_mutex;

int main(void)
{
	bar->set_done_char("=");

	std::vector<std::string> original;//입력
	std::vector<std::string> answer_sheet;//답지

	for (int i = 0; i < kMaxProduce; i++) {//workitem과 정답지 생성
		original.push_back(generate_random_string());
		answer_sheet.push_back(original[i]);
		std::sort(answer_sheet[i].begin(), answer_sheet[i].end());
	}

	AsyncQueue<std::unique_ptr<WorkItem> > work_item_queue;
	work_item_queue.setCapacity(kMaxQueueCapa);//producer와 consumer간 workitem보관 queue크기 제한.

	printf("start\n");
	std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();

	std::atomic_int consumed_count = 0;
	std::atomic_int overflow_count = 0;
	std::unique_ptr<WorkerThread> consumer_thread = std::make_unique<WorkerThread>();//workitem을 처리하는 consumer thread
	if (consumer_thread) {
		consumer_thread->setThreadBody(
			"Consumer",
			[&]() {
			while ((consumed_count + overflow_count) < kMaxProduce) {
				std::unique_ptr<WorkItem> workitem = work_item_queue.tryPop();
				if (!workitem) {
					continue;
				}
				user_consume(std::move(workitem));//<<- 사용자 구현 함수 2
				consumed_count++;
			}});
	}
	std::unique_ptr<WorkerThread> producer_thread = std::make_unique<WorkerThread>();//workitem을 생성하는  producer thread
	if (producer_thread) {
		producer_thread->setThreadBody(
			"Producer",
			[&]() {
			for (int i = 0; i < kMaxProduce; i++) {
				bool is_overflow = work_item_queue.push(
					std::make_unique<WorkItem>(i, original[i])
				);
				if (is_overflow) {
					overflow_count++;
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}});
	}

	user_init(kMaxProduce); //<<- 사용자 구현 함수 1

	if (consumer_thread) {
		consumer_thread->start();
	}
	if (producer_thread) {
		producer_thread->start();
	}

	if (producer_thread) {
		producer_thread->stop();
	}
	if (consumer_thread) {
		consumer_thread->stop();
	}

	user_deinit(); //<<- 사용자 구현 함수 3

	auto end_time = std::chrono::steady_clock::now();
	std::chrono::milliseconds chrono_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
	printf("\nchrono_duration<ms>:%lld\n", chrono_duration.count());


	int score = (int)chrono_duration.count()*10;

	score += (overflow_count * 5000);
	score += (int)(original.size() - processed.size()) * 100000;


	int correct_count = 0;
	for( std::map<int, std::string>::iterator iter = processed.begin(); iter != processed.end(); ++iter){
		int idx = iter->first;
		if(iter->second != answer_sheet[idx]) {
			score += 100000;
		}
		else {
			correct_count++;
		}
	}
	printf("consumed/produced: %d/%d\n", consumed_count.load(), (int)original.size());
	printf("processed/produced: %d/%d\n", (int)processed.size(), (int)original.size());
	printf("correct/processed: %d/%d\n", correct_count, (int)processed.size());
	printf("end with score: %d\n", score);
	return 0;
}

std::string generate_random_string()
{
	std::vector<char> alphabet = { 'a', 'b', 'c', 'd', 'e', 'f', 'g',
						  'h', 'i', 'j', 'k', 'l', 'm', 'n',
						  'o', 'p', 'q', 'r', 's', 't', 'u',
						  'v', 'w', 'x', 'y', 'z' };
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dis_len(10, 100);

	int len = dis_len(gen);

	std::uniform_int_distribution<int> dis_alphabet(0, 25);
	std::string res;
	for (int i = 0; i < len; i++) {
		res = res + alphabet[dis_alphabet(gen)];
	}
	return res;
}