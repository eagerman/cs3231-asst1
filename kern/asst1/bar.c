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

#define  FREE 0
#define  BUSY 1


/* Declare any globals you need here (e.g. locks, etc...) */
static int bottle_states[NBOTTLES+1]; // the state of every bottle (BUSY/FREE)
static struct semaphore *mutex; // sem to insure mutual exclusion
// static struct semaphore *bartender_sems[NBARTENDERS]; functionality of this arr is placed in the DLL
static OrderList pending_list; // this will never have more than NCUSTOMERS elements (theoritically at least)
static struct semaphore *orders_pending;
static struct semaphore *list_mutex;


/*
 * **********************************************************************
 * FUNCTION HEADERS
 * **********************************************************************
 */

int test(node *order);

OrderList createList(void);

node *addOrder(OrderList list, struct barorder *order);

void removeOrder(OrderList list, node *order);

node *what_can_I_make_now(OrderList list);

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
        // the addOrder function should have set up the order struct as needed
        // for this implementation to work
        addOrder(pending_list, order);
        // signals that there are pending orders
        V(orders_pending);
        
        // block until signalled as done
        P(order->done);
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
        //TODO
        // block until we are signalled that an order is submitted
        P(orders_pending);
        // traverse list checking whether we can make a drink
        // we're going head---->tail to prioritize orders made first
        node *curr;
        while(TRUE) {
                
                for (curr = pending_list->head; curr != NULL; curr = curr->next) {
                        if (test(curr)) {
                                V(list_mutex);
                                return curr;
                        }
                }
                
                
        }
        return NULL;
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
        
        int i = 0;
        for (; i < DRINK_COMPLEXITY; i++) {
                bottle_states[order->requested_bottles[i]] = FREE;
        }
}


/*
 * serve_order()
 *
 * Takes a filled order and makes it available to (unblocks) the
 * waiting customer.
 */

void serve_order(struct barorder *order)
{
        V(order->done);
        
        sem_destroy(order->done);
        sem_destroy(order->bartender_sem);
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
        pending_list = createList();
        mutex = sem_create("bar.c: mutex", 1);
        if (mutex == NULL)
                panic("bar.c: mutex failed to allocate!\n");
        orders_pending = sem_create("bar.c: orders_pending", 0);
        if (orders_pending == NULL)
                panic("bar.c: orders_pending failed to allocate!\n");
        list_mutex = sem_create("bar.c: list_mutex", 1);      
        if (mutex == NULL)
                panic("bar.c: list mutex failed to allocate!\n");
        int i;
        for (i = 0; i <= NBOTTLES; i++) {
                bottle_states[i] = FREE;
        }

}

/*
 * bar_close()
 *
 * Perform any cleanup after the bar has closed and everybody
 * has gone home.
 */
// TODO be more rigorous in cleaning up
void bar_close(void)
{
        kfree (pending_list);
        sem_destroy(mutex);
        sem_destroy(list_mutex);
        sem_destroy(orders_pending);
}


/*
 * **********************************************************************
 * HELPER FUNCTIONS
 * **********************************************************************
 */
 
 /*
 * test(node *o)
 *
 * test to see if the order can be fulfilled at this moment
 * if so, update the bottle state(s) and inform bartender to complete this order
 */
 
int test(node *order) {
        // TODO ensure mutual exclusion for altering states
        
        int all_free = TRUE;
        int i;
        // entering critical section
        for (i = 0; i < DRINK_COMPLEXITY; i++) {
                if (bottle_states[order->requested_bottles[i]] == BUSY)
                        all_free = FALSE;
        }
        
        if (all_free) {
                for (i = 0; i < DRINK_COMPLEXITY; i++) {
                        if (order->requested_bottles[i] == 0) continue; //empty
                        bottle_states[order->requested_bottles[i]] = BUSY;
                }
                
                order->state = BUSY;
                removeOrder(pending_list, order);
                return TRUE;
        } else {
                return FALSE;
        }
        
}

/*
 * **********************************************************************
 * OrderList MANAGEMENT FUNCTIONS
 * **********************************************************************
 */
 
OrderList createList() {
        OrderList new = kmalloc(sizeof(struct OL));
        if (new == NULL) {
                panic("bar.c:createList(): kmalloc failed\n");
        }
        new->head = NULL;
        new->tail = NULL;
        return new;
}
 
node *addOrder(OrderList list, struct barorder *order) {
        P(list_mutex);
        if (list->tail == NULL) {
                list->tail = order;
                list->head = order;
                                
                order->next = NULL;
                order->prev = NULL;                
        } else {
                list->tail->next = order;

                
                order->prev = list->tail;
                list->tail = order;
                
                
                order->next = NULL;
        }
        
        order->bartender_sem = sem_create("bartender_sem", 0);
        order->done = sem_create("done", 0);
        V(list_mutex);
        
        
        if (order->bartender_sem == NULL) 
                panic("bar.c: addOrder: bartender_sem is null!\n");
        if (order->done == NULL) 
                panic("bar.c: addOrder: done is null!\n");
        return order;
}

// removes the order from the list
void removeOrder(OrderList list, node *order) {
        P(list_mutex);
        node *prev_node = order->prev;
        node *next_node = order->next;
        
        if (order == list->head && order == list->tail) {
                list->head = NULL;
                list->tail = NULL;
        } else if (order == list->head) {
                list->head = next_node;
                list->head->prev = NULL;
        } else if (order == list->tail) {
                list->tail = prev_node;
                list->tail->next = NULL;
        } else {
                prev_node->next = next_node;
                next_node->prev = prev_node;
        }
        V(list_mutex);
        // take care of mem leaks
        //sem_destroy(order->bartender_sem);
        //sem_destroy(order->done);
        //kfree(order);
        
}

/*node *what_can_I_make_now(OrderList list) {
        node *curr = list->head;
        for(; curr != NULL; curr = curr->next) {
                if (test(curr)) {
                        return curr;
                }
        } 
        return NULL;
      
}*/
