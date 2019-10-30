# async.hpp - asynchronous, stackless coroutines for C++

Single header implementation of async/await concept - reformulation of my [cpp-generators](https://github.com/c-smile/cpp-generators) and [async.h](https://github.com/naasking/async.h) but for C++ this time.

Here is MOVIE of async.hpp running on board of my [Arduino based test platform "Vasilii"](https://terrainformatica.com/2018/10/10/refreshing-ai-basics/)

[![async.hpp in action](https://img.youtube.com/vi/aQPYnlXV3ZY/maxresdefault.jpg)](https://youtu.be/aQPYnlXV3ZY)

# Features

1. It's 100% pure, portable C++.
2. It's not dependent on an OS. Can be used even without OS, e.g. on Arduino.

# Basics

async.hpp is all about `async::task` - a structure holding state of coroutine and its bool run() method. 

The task is a statefull function that can be entered and exited "in the middle". Next its `run()` invocation will continue at the statment when the coroutine issued `yield` (and its variants) last time.

Pseudo code

```
    struct task {
      bool flip_flop = false;
      void run() {
        flip_flop = true;
        sleep(20);
        flip_flop = false;
        sleep(20);
      }
      bool operator()() { run(); return is_done(); }
    }

    task flip_flop_task;
```
As soon as the task will be called in a loop:

```C++
    void main() {

       while(true) {
         flip_flop_task();
       }

    }
```

It will pulse its flip_flop state each 40 ms. 


# API

Function|Description
--------|-----------
*TASK(name) {...}*|Declaration of the task - async::task derived (CRTP) structure
*TASK_BEGIN*|Mark beginning of code section of the coroutine (its `run()` method)
*TASK_END*|Mark end of ode section of the coroutine.
*TASK_STOP*|Stop execution of the task, a.k.a. async "return".
*YIELD*|Yield execution until it's invoked again
*AWAIT(cond)*|Block execution until `cond` is true
*START_TASK(name,params...)*|Initialize task state and start its execution
*RUN_TASK(name,params...)*|Run task and wait until it ends

Auxiliary primitives

Function|Description
--------|-----------
*async::timer*|waitable timer
*async::semaphore*|semaphore/event 

# Examples

Ported example of async.h:
```C++
#include "async.hpp"

TASK(example) {

    async::timer timer; // state variable
    
    TASK_BEGIN
    
    while(true) {
        if(initiate_io()) {
            timer.start(200);
            AWAIT(io_completed() || timer.is_expired());
            read_data();
        }
    }
    
    TASK_END
}
```


## Nested Async Calls

You can also execute nested async tasks. Here is an example of my Arduino robot implementation :

```C++
TASK(rollback_task) {

  int angle_to_turn; // state variable

public:
  
  void init(int angle = 15) { angle_to_turn = angle; }
  
  TASK_BEGIN

    engine.set(-0.4);  // move backwards in slow speed
    SLEEP(1200);       // for 1.2 seconds
    // run task of turning the device to the given angle:  
    RUN_TASK(turn_to_task, random(2)? -angle_to_turn: angle_to_turn);
    engine.stop();     // stop the engine  

  TASK_END

} rollback_task;

```

The whole robot's code is just a composition of such tasks, some of them run in parallel, some one by one. There is no OS, no threads, on the device just a `loop()` routine called by the processor. The async.hpp allows to implement parallelism and asynchronous execution in 8kb of RAM the device has on board.

# Caveats

1. You cannot use switch statements within an async subroutine - execution 
   state management is a switch itself.
2. You cannot make blocking system calls. Either use poll() or completion I/O primitives.

# Build samples in /code folder

There is premake5 file there. So you can generate VS, XCode, etc. projects:

1. Download [premake5](https://premake.github.io/download.html)
2. run `premake5.exe vs2015` in the /code folder to generate VS2015 project and workspace.

# Practical usage

I am using [cpp-generators](https://github.com/c-smile/cpp-generators) and the async::task (combined with libuv for I/O) intensively in my [Sciter Engine](https://sciter.com). You can find [Sciter SDK](https://github.com/c-smile/sciter-sdk) on GitHub too. This allows me to keep Sciter small, fast and manageable.

As of the robot pet project, read my [Refreshing AI basics …](https://terrainformatica.com/2018/10/10/refreshing-ai-basics/) article. 
