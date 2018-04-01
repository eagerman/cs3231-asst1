#ifndef BAR_H
#define BAR_H
#include <synch.h>

#include "barglass.h"

/*
 * You are free to add anything you think you require to this file,
 * with the exceptions noted below.
 */


/* struct barorder is the main type referred to in the code. It must
   be preserved as noted for our later testing to work */

struct barorder {
        unsigned int requested_bottles[DRINK_COMPLEXITY]; /* Do not change */
        int go_home_flag;                                 /* Do not change */
        struct glass glass;                               /* Do not change */

        /* This struct can be extended with your own entries below here */ 

        /* barorder is a Doubly-Linked List*/
        /* end nodes are null nodes */
        struct barorderNode *next;
        struct barorderNode *prev;
        struct semaphore *bartender_sem;
        struct semaphore *done;
        int    state; // this order is awaiting fulfillment or not picked up yet


};

typedef struct barorder node; // A bar order is a node

// list handle (to avoid O(n) insertion and deletion)
typedef struct OL {
        node *head;
        node *tail;
} *OrderList;

#endif
