#include "workitem.h"
#include "workerthread.h"
#include <future>


void user_init(int max_produce_)
{
  //TODO: 이부분에는 사전 동작을 정의하면됩니다. 
}

void user_consume(std::unique_ptr<WorkItem> workitem)
{
  //TODO: 이부분을 수정해서 처리속도를 빠르게 하면됩니다. 
  workitem->process();
}

void user_deinit(void)
{
  //TODO: 이부분에는 사후 동작을 정의하면됩니다. 
}


