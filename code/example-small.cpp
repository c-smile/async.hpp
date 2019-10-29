#include "async.hpp"

#include <stdio.h> /* For printf(). */

/* Two flags that the two async functions use. */
static bool first_flag = true, second_flag = false;

TASK(first) {

  /* Here is the place of declarations of state variables that shall persist between runs */
  int n = 0;

  /* An async function body must be enclosed into begin / end - defines run() method */
  TASK_BEGIN

    /* We loop forever here. */
    while (true) {
      /* Wait until the other async has set its flag. */
      AWAIT(second_flag);
      printf("async 1 run # %d\n", ++n);

      /* We then reset the other async's flag, and set our own
      flag so that the other async can run. */
      second_flag = false;
      first_flag = true;

      /* And we loop. */
    }

  /* All async functions must end with async_end which takes a
  pointer to a struct pt. */
  TASK_END
};

TASK(second) {
  /* A async function must begin with async_begin() which takes a
  pointer to a struct async. */

  TASK_BEGIN

    /* We loop forever here. */
    while (true) {
      /* Wait until the other async has set its flag. */
      AWAIT(first_flag);
      printf("async 2 running\n");

      /* We then reset the other async's flag, and set our own
      flag so that the other async can run. */
      first_flag = false;
      second_flag = true;

      /* And we loop. */
    }

  /* All async functions must end with async_end which takes a
  pointer to a struct pt. */
  TASK_END
};

void example_small(int i) {
  first n1;
  second n2;

  while (--i >= 0) {
    n1();
    n2();
  }

}