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

static struct pc_data buffer[BUFFER_SIZE]; // the buffer (queue)
static int head; // index of the head of the queue
static int tail; // index of the tail of the queue
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
        P(full); // consumer will block if the fill count is zero
        P(mutex); // this is the buffer semaphore, it is used to prevent multiple consumers consuming the same item concurrently
        
        // the data to be consumed is at the head of the buffer
        thedata = buffer[head];
        // the shifts one step closer to the tail
        head    = (head + 1) % BUFFER_SIZE;

        V(mutex); //release buffer
        V(empty); // signal to waiting producers that an empty slot in the buffer is now available

        return thedata;
}

/* procucer_send() is called by a producer to store data in your
   bounded buffer. */

void producer_send(struct pc_data item)
{
        P(empty); // producer will block once the empty count is zero, i.e buffer is full
        P(mutex); // buffer semaphore
        
        // tail "extends" and wraps around (if reached end)
        tail = (tail + 1) % BUFFER_SIZE;
        // set argument (item) as the new tail
        buffer[tail] = item;

        V(mutex); // release buffer
        V(full); // signal to waiting consumers that an item has been added to the buffer
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
        head = 0; // the queue is initially empty
        tail = -1;
}

/* Perform any clean-up you need here */
void producerconsumer_shutdown(void)
{
        sem_destroy(empty);
        sem_destroy(full);
        sem_destroy(mutex);
}

