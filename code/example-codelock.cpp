/*
 * Copyright (c) 2004-2005, Swedish Institute of Computer Science.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 * 3. Neither the name of the Institute nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software 
 *    without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE. 
 *
 * This file is part of the async.h library.
 *
 * Author: Adam Dunkels <adam@sics.se>, Sandro Magi <naasking@gmail.com>
 *
 * $Id: example-codelock.c,v 1.5 2005/10/06 07:57:08 adam Exp $
 */

/*
 *
 * This example shows how to implement a simple code lock. The code
 * lock waits for key presses from a numeric keyboard and if the
 * correct code is entered, the lock is unlocked. There is a maximum
 * time of one second between each key press, and after the correct
 * code has been entered, no more keys must be pressed for 0.5 seconds
 * before the lock is opened.
 *
 * This is an example that shows two things:
 * - how to implement a code lock key input mechanism, and
 * - how to implement a sequential timed routine.
 *
 * The program consists of two async functions, one that implements the
 * code lock reader and one that implements simulated keyboard input.
 *
 *
 */

#include <stdio.h>

#include "async.hpp"

static async::timer codelock_timer, input_timer;
/*---------------------------------------------------------------------------*/
/*
 * This is the code that has to be entered.
 */
static const char code[4] = {'1', '4', '2', '3'};

/*---------------------------------------------------------------------------*/
/*
 * The following code implements a simple key input. Input is made
 * with the press_key() function, and the function key_pressed()
 * checks if a key has been pressed. The variable "key" holds the
 * latest key that was pressed. The variable "key_pressed_flag" is set
 * when a key is pressed and cleared when a key press is checked.
 */
static char key, key_pressed_flag;

static void
press_key(char k)
{
  printf("--- Key '%c' pressed\n", k);
  key = k;
  key_pressed_flag = 1;
}

static bool
key_pressed(void)
{
  if(key_pressed_flag != 0) {
    key_pressed_flag = 0;
    return true;
  }
  return false;
}
/*---------------------------------------------------------------------------*/
/*
 * Declaration of the async function implementing the code lock
 * logic. The async function is declared using the `async` return type.
 * The function is declared with the "static" keyword since it
 * is local to this file. The name of the function is codelock_thread
 * and it takes one argument, pt, of the type struct async.
 *
 */
TASK(codelock_thread)
{
  /* This is a local variable that holds the number of keys that have
   * been pressed. Note that it is declared with the "static" keyword
   * to make sure that the variable is *not* allocated on the stack.
   */
  int keys;

  /*
   * Declare the beginning of the async.
   */
  TASK_BEGIN

  /*
   * We'll let the async loop until the async is
   * expliticly exited with async_exit.
   */
  while(1) {

    /*
     * We'll be reading key presses until we get the right amount of
     * correct keys.
     */ 
    for(keys = 0; keys < sizeof(code); ++keys) {

      /*
       * If we haven't gotten any keypresses, we'll simply wait for one.
       */
      if(keys == 0) {
	      /*
	       * The AWAIT() function will block until the condition
	       * key_pressed() is true.
	       */
	      AWAIT(key_pressed());
      } else {
	
	/*
	 * If the "key" variable was larger than zero, we have already
	 * gotten at least one correct key press. If so, we'll not
	 * only wait for the next key, but we'll also set a timer that
	 * expires in one second. This gives the person pressing the
	 * keys one second to press the next key in the code.
	 */
 	     codelock_timer.start(1000);

	/*
	 * The following statement shows how complex blocking
	 * conditions can be easily expressed with asyncs and
	 * the AWAIT() function.
	 */
	     AWAIT(key_pressed() || codelock_timer.is_expired());

	/*
	 * If the timer expired, we should break out of the for() loop
	 * and start reading keys from the beginning of the while(1)
	 * loop instead.
	 */
	    if(codelock_timer.is_expired()) {
	      printf("Code lock timer expired.\n");	  
	        /*
	         * Break out from the for() loop and start from the
	         * beginning of the while(1) loop.
	         */
	        break;
	      }
      }

      /*
       * Check if the pressed key was correct.
       */
      if(key != code[keys]) {
	      printf("Incorrect key '%c' found\n", key);
	      /*
	       * Break out of the for() loop since the key was incorrect.
	       */
	      break;
      } else {
	      printf("Correct key '%c' found\n", key);
      }
    }

    /*
     * Check if we have gotten all keys.
     */
    if(keys == sizeof(code)) {
      printf("Correct code entered, waiting for 500 ms before unlocking.\n");

      /*
       * Ok, we got the correct code. But to make sure that the code
       * was not just a fluke of luck by an intruder, but the correct
       * code entered by a person that knows the correct code, we'll
       * wait for half a second before opening the lock. If another
       * key is pressed during this time, we'll assume that it was a
       * fluke of luck that the correct code was entered the first
       * time.
       */
      codelock_timer.start(500);      
      AWAIT(key_pressed() || codelock_timer.is_expired());

      /*
       * If we continued from the AWAIT() statement without
       * the timer expired, we don't open the lock.
       */
      if(!codelock_timer.is_expired()) {
	      printf("Key pressed during final wait, code lock locked again.\n");
      } else {
	      /*
	       * If the timer expired, we'll open the lock and exit from the
	       * async.
	       */
	      printf("Code lock unlocked.\n");
        TASK_STOP;
      }
    }
  }

  /*
   * Finally, we'll mark the end of the async.
   */
  TASK_END
};
/*---------------------------------------------------------------------------*/
/*
 * This is the second async in this example. It implements a
 * simulated user pressing the keys. This illustrates how a linear
 * sequence of timed instructions can be implemented with
 * asyncs.
 */
TASK(input_thread)
{
  TASK_BEGIN

    printf("Waiting 1 second before entering first key.\n");

  input_timer.start(1000);
  AWAIT(input_timer);

  press_key('1');

  input_timer.start(100);
  AWAIT(input_timer);

  press_key('2');

  input_timer.start(100);
  AWAIT(input_timer);

  press_key('3');

  input_timer.start(2000);
  AWAIT(input_timer);

  press_key('1');

  input_timer.start(200);
  AWAIT(input_timer);

  press_key('4');

  input_timer.start(200);
  AWAIT(input_timer);

  press_key('2');

  input_timer.start(2000);
  AWAIT(input_timer);

  press_key('3');

  input_timer.start(200);
  AWAIT(input_timer);

  press_key('1');

  input_timer.start(200);
  AWAIT(input_timer);

  press_key('4');

  input_timer.start(200);
  AWAIT(input_timer);

  press_key('2');

  input_timer.start(100);
  AWAIT(input_timer);

  press_key('3');

  input_timer.start(100);
  AWAIT(input_timer);

  press_key('4');

  input_timer.start(1500);
  AWAIT(input_timer);

  press_key('1');

  input_timer.start(300);
  AWAIT(input_timer);

  press_key('4');

  input_timer.start(400);
  AWAIT(input_timer);

  press_key('2');

  input_timer.start(500);
  AWAIT(input_timer);

  press_key('3');

  input_timer.start(2000);
  AWAIT(input_timer);

  TASK_END
};
/*---------------------------------------------------------------------------*/
/*
 * This is the main function. It initializes the two async
 * control structures and schedules the two asyncs. The main
 * function returns when the async the runs the code lock exits.
 */
int example_codelock(void)
{
  /*
   * Initialize the two async control structures.
   */
  input_thread input;
  codelock_thread codelock;

  /*
   * Schedule the two asyncs until the codelock_thread() exits.
   */
  while(!codelock()) {
    input();
    
    /*
     * When running this example on a multitasking system, we must
     * give other processes a chance to run too and therefore we call
     * usleep() resp. Sleep() here. On a dedicated embedded system,
     * we usually do not need to do this.
     */
#ifdef _WIN32
    Sleep(0);
#else
    usleep(10);
#endif
  }

  return 0;
}
