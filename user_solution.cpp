#include "workitem.h"
#include "workerthread.h"
#include <future>


void user_init(int max_produce_)
{
  //TODO: �̺κп��� ���� ������ �����ϸ�˴ϴ�. 
}

void user_consume(std::unique_ptr<WorkItem> workitem)
{
  //TODO: �̺κ��� �����ؼ� ó���ӵ��� ������ �ϸ�˴ϴ�. 
  workitem->process();
}

void user_deinit(void)
{
  //TODO: �̺κп��� ���� ������ �����ϸ�˴ϴ�. 
}


