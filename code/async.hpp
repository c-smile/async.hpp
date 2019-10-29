#pragma once

#ifndef __ASYNC_HPP__
#define __ASYNC_HPP__

#if defined(_WIN32)
  #include <windows.h>
#elif defined(ARDUINO)
  #include <unistd.h>
#else /* _WIN32 */
  #include <unistd.h>
  #include <sys/time.h>
#endif /* _WIN32 */

namespace async {

  inline unsigned long clock_time();

  //|
  //| async timer
  //| 
  class timer 
  { 
    unsigned long until;
    timer(const timer& other) = delete;
  public:
    timer() :until(0) {}

    void start(unsigned int duration) { until = clock_time() + duration; }
    bool is_expired() { return clock_time() >= until; }
  };


  //|
  //| The task - core async class, represents async function with persistent state 
  //|            between calls. A.k.a. corroutine, async routine
  //|

  template<class TIMPL> 
  class task {
  protected:

    // task status, contains either the below values or line number where the task is at the moment
    enum status : int { INIT = 0, DONE = -1 };

    status __status; 
    timer  __sleep_timer;

    // non-copyable
    task(const task&) = delete;
    task &operator=(const task &) = delete;

  public:

    task() { restart(); }

    //|
    //| Restart, a.k.a. rewind 
    //|
    void restart() { __status = INIT; }

    //| each task may have its own init() with its own signature 
    //| used by START/RUN_TASK() to pass parameters to initial state
    void init() {}
    
    //|
    //| Check if async subroutine is done
    //|
    bool is_done() const { return __status == DONE; }

    //|
    //| resume a running async computation and check for completion
    //|
    bool operator()() {
      if(is_done()) return false;
      __status = static_cast<TIMPL *>(this)->run();
      return is_done();
    }
  };

  //|
  //|  Task helper declaration, note it is CRTP thing, see: https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
  //|
  #define TASK(NAME) class NAME : public async::task<NAME>

  //|
  //| Mark the start of the async routine, defines async::status run() method
  //|
  #define TASK_BEGIN public: status run() { switch(__status) { case 0:

  //|
  //| Mark the end of 
  //| 
  #define TASK_END default: return (__status = DONE); } }

  //|
  //| Wait until the condition succeeds
  //|
  #define AWAIT(cond) case __LINE__: if (async::is_incomplete(cond)) return status(__LINE__)

  //|
  //| Yields execution
  //|
  #define YIELD return status(__LINE__); case __LINE__:

  //|
  //| Sleeps the task for that amount of milliseconds
  //|
  #define SLEEP(ms) __sleep_timer.start(ms); AWAIT(__sleep_timer)

  //|
  //| Exit the current async subroutine
  //|
  #define TASK_STOP return (__status = DONE);

  //|
  //| start task and wait for its completion
  //|
  #define RUN_TASK(T, ...)  do { T.restart(); T.init(__VA_ARGS__); AWAIT(T); } while(false)  


  //|
  //| async semaphore
  //|
  class semaphore 
  {
    semaphore(const semaphore& other) = delete;
    unsigned int _count;
    friend bool is_incomplete(semaphore& c);
    bool is_incomplete() {
      if (_count == 0) return true;
      --_count;
      return false;
    }
  public:
    semaphore(unsigned int c = 0) : _count(c) {}

    void signal() { ++_count; }

    void reset(unsigned int n) { _count = n; }
  };

  //|
  //| current number of ticks since system start
  //|
  inline unsigned long clock_time() {
#if defined(ARDUINO)
      return millis();
#elif defined(_WIN32)
      return GetTickCount();
#else
      struct timeval tv;
      struct timezone tz;
      gettimeofday(&tv, &tz);
      return unsigned(tv.tv_sec * 1000 + tv.tv_usec / 1000);
#endif
  }

  // adaptors for AWAIT 
  inline bool is_incomplete(bool c) { return !c; }
  inline bool is_incomplete(timer& c) { return !c.is_expired(); }  
  inline bool is_incomplete(semaphore& c) { return c.is_incomplete(); }
  template<typename T> inline bool is_incomplete(task<T>& t) { t.operator()(); return !t.is_done(); }
}

#endif
