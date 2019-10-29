#include "async.hpp"

#include <stdio.h> /* For printf(). */

int  example_buffer(void);
int  example_codelock(void);
void example_small(int i);

int main(void) {
  example_small(200);
  example_buffer();
  example_codelock();
  return 0;
}

