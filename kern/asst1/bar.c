#include <types.h>
#include <lib.h>
#include <synch.h>
#include <test.h>
#include <thread.h>

#include "bar.h"
#include "bar_driver.h"



/*
 * **********************************************************************
 * YOU ARE FREE TO CHANGE THIS FILE BELOW THIS POINT AS YOU SEE FIT
 *
 */

#define FALSE 0
#define  TRUE 1

#define FREE  0
#define BUSY  1


/* Declare any globals you need here (e.g. locks, etc...) */
static int bottle_states[NBOTTLES]; // the state of every bottle (BUSY/FREE)
static struct semaphore *mutex; // sem to insure mutual exclusion
// static struct semaphore *bartender_sems[NBARTENDERS]; functionality of this arr is placed in the DLL
static OrderList pending_list; // this will never have more than NCUSTOMERS elements (theoritically at least)

/*
 * **********************************************************************
 * FUNCTIONS EXECUTED BY CUSTOMER THREADS
 * **********************************************************************
 */

/*
 * order_drink()
 *
 * Takes one argument referring to the order to be filled. The
 * function makes the order available to staff threads and then blocks
 * until a bartender has filled the glass with the appropriate drinks.
 */

void order_drink(struct barorder *order)
{
        (void) order; /* Avoid compiler warning, remove when used */
        panic("You need to write some code!!!!\n");
}



/*
 * **********************************************************************
 * FUNCTIONS EXECUTED BY BARTENDER THREADS
 * **********************************************************************
 */

/*
 * take_order()
 *
 * This function waits for a new order to be submitted by
 * customers. When submitted, it returns a pointer to the order.
 *
 */

struct barorder *take_order(void)
{
        struct barorder *ret = NULL;

        return ret;
}


/*
 * fill_order()
 *
 * This function takes an order provided by take_order and fills the
 * order using the mix() function to mix the drink.
 *
 * NOTE: IT NEEDS TO ENSURE THAT MIX HAS EXCLUSIVE ACCESS TO THE
 * REQUIRED BOTTLES (AND, IDEALLY, ONLY THE BOTTLES) IT NEEDS TO USE TO
 * FILL THE ORDER.
 */

void fill_order(struct barorder *order)
{

        /* add any sync primitives you need to ensure mutual exclusion
           holds as described */

        /* the call to mix must remain */
        mix(order);

}


/*
 * serve_order()
 *
 * Takes a filled order and makes it available to (unblocks) the
 * waiting customer.
 */

void serve_order(struct barorder *order)
{
        (void) order; /* avoid a compiler warning, remove when you
                         start */
}



/*
 * **********************************************************************
 * INITIALISATION AND CLEANUP FUNCTIONS
 * **********************************************************************
 */


/*
 * bar_open()
 *
 * Perform any initialisation you need prior to opening the bar to
 * bartenders and customers. Typically, allocation and initialisation of
 * synch primitive and variable.
 */

void bar_open(void)
{

}

/*
 * bar_close()
 *
 * Perform any cleanup after the bar has closed and everybody
 * has gone home.
 */

void bar_close(void)
{

}


/*
 * **********************************************************************
 * HELPER FUNCTIONS
 * **********************************************************************
 */
 
 /*
 * test(OrderList *o)
 *
 * test to see if the order can be fulfilled at this moment
 * if so, update the bottle state(s) 
 */
 
int test(OrderList order) {
        // TODO ensure mutual exclusion for altering states
        
        int all_free = TRUE;
        int i;
        // entering critical section
        P(mutex);
        for (i = 0; i < DRINK_COMPLEXITY; i++) {
                if (bottle_states[order->order->glass.contents[i]] == BUSY)
                        all_free = FALSE;
        }
        
        if (all_free) {
                for (i = 0; i < DRINK_COMPLEXITY; i++) {
                        bottle_states[order->order->glass.contents[i]] = BUSY;
                }
                
                V(mutex);
                V(order->bartender_sem);
        } else {
                V(mutex);
        }
        
        return TRUE;
}

/*
 * **********************************************************************
 * OrderList MANAGEMENT FUNCTIONS
 * **********************************************************************
 */
 
OrderList create_list() {
        return NULL;
}
 
OrderList addOrder(OrderList list, struct barorder *order) {
        OrderList ret;
        if (list == NULL) {
              ret = kmalloc(sizeof(struct barorderNode));
              ret->next = NULL;
              ret->prev = NULL;
              ret->order = order;
              ret->bartender_sem = sem_create("bartender_sem", 0);
              //ret->state = ??? //TODO
              
              return ret;
        }
        // else list is already populated
        
        OrderList curr = list;
        
        for (curr = list; curr->next != NULL; curr = curr->next);
        OrderList tail = curr;
        curr->next = kmalloc(sizeof(struct barorderNode));
        curr = curr->next;
        curr->next = NULL;
        curr->prev = tail;
        tail->next = curr;
        
        curr->order = order;
        curr->bartender_sem = sem_create("bartender_sem", 0);
        
        return curr;
}

void removeOrder(OrderList order) {
        OrderList prev_node = order->prev;
        OrderList next_node = order->next;
        if (prev_node != NULL) {
                prev_node->next = next_node;
        }
        if (next_node != NULL) {
                next_node->prev = prev_node;
        }
        
        // take care of mem leaks
        sem_destroy(order->bartender_sem);
        kfree(order);
        
}
