#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <stdio.h>

#include "async.hpp"

#include <vector>
#include <memory>

TASK(task_list)
{
  unsigned half = 0;
  std::vector<std::shared_ptr<async::job>> lists[2];

  std::vector<std::shared_ptr<async::job>>& active_list() { return lists[half & 1]; }
  std::vector<std::shared_ptr<async::job>>& passive_list() { return lists[(half + 1) & 1]; }

public:
  void add_task(std::shared_ptr<async::job> task) {
    active_list().push_back(task);
  }
  unsigned active_tasks() {  return (unsigned) active_list().size(); }

  TASK_BEGIN

    for(;;) {
      passive_list().clear();
      for (auto t : active_list()) {
        if (t->operator()())
          continue;
        passive_list().push_back(t);
      }
      ++half; // swap list roles
      YIELD;
    }

  TASK_END
} task_list_instance;

void add_task(std::shared_ptr<async::job> task) {
  task_list_instance.add_task(task);
}

void heartbit() {
  task_list_instance.operator()();
}

unsigned active_tasks() {
  return task_list_instance.active_tasks();
}

#define FIRE_TASK(T, ...)  add_task( std::make_shared<T>(__VA_ARGS__))

//SLEEP

TASK(n1)
{
    int c;
public:
    n1(int count) : c(count) {}
    ~n1() { printf("n1 - done\n"); }

  TASK_BEGIN

    for (; c >= 0; --c) {
      SLEEP(100);
      printf("n1 c=%d\n", c);
    }

  TASK_END
};

TASK(n2)
{
   int c;
public:
   n2(int count) : c(count) {}
   ~n2() { printf("n2 - done\n"); }

  TASK_BEGIN

    for (; c >= 0; --c) {
      SLEEP(113);
      printf("n2 c=%d\n", c);
    }

  TASK_END
};

void example_dyn_task() {

  FIRE_TASK(n1, 12);
  FIRE_TASK(n2, 23);

  while (active_tasks()) {
    heartbit();
  }

  printf("done dynamic tasks\n");

}


