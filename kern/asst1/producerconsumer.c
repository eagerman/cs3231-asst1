/* This file will contain your solution. Modify it as you wish. */
#include <types.h>
#include "producerconsumer_driver.h"
#include "opt-synchprobs.h"
#include <lib.h>    /* for kprintf */
#include <synch.h>  /* for P(), V(), sem_* */
#include <thread.h> /* for thread_fork() */
//#include <test.h>

/* Declare any variables you need here to keep track of and
   synchronise your bounded. A sample declaration of a buffer is shown
   below. You can change this if you choose another implementation. */

static struct pc_data buffer[BUFFER_SIZE];
static int head;
static int tail;
static struct semaphore *empty; // starts out with BUFFER_SIZE
static struct semaphore *full; // starts out with 0
static struct semaphore *mutex; // self descriptive


/* consumer_receive() is called by a consumer to request more data. It
   should block on a sync primitive if no data is available in your
   buffer. */

struct pc_data consumer_receive(void)
{
        struct pc_data thedata;

        /* FIXME: this data should come from your buffer, obviously... */
        P(full);
        P(mutex);
        
        thedata = buffer[head];
        head    = (head + 1) % BUFFER_SIZE;
        
        V(mutex);
        V(empty);

        return thedata;
}

/* procucer_send() is called by a producer to store data in your
   bounded buffer. */

void producer_send(struct pc_data item)
{
        P(empty);
        P(mutex);
        
        tail = (tail + 1) % BUFFER_SIZE;
        buffer[tail] = item;
        
        V(mutex);
        V(full);
}




/* Perform any initialisation (e.g. of global data) you need
   here. Note: You can panic if any allocation fails during setup */

void producerconsumer_startup(void)
{
        empty = sem_create("empty", BUFFER_SIZE);
        full  = sem_create("full", 0);
        mutex = sem_create("mutex", 1);
        if (empty == NULL || full == NULL)
                panic("producerconsumer_startup: can't allocate semaphores!\n");
        head = 0;
        tail = 0;
}

/* Perform any clean-up you need here */
void producerconsumer_shutdown(void)
{
        sem_destroy(empty);
        sem_destroy(full);
        sem_destroy(mutex);
}

