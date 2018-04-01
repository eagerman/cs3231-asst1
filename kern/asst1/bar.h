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

};

/* struct barorderNode is a Doubly Linked List data structure
   to hold pending orders for bar.c, there is no notion of size.
   Ends of the list are null. */
struct barorderNode {
        struct barorder *order;
        struct barorderNode *next;
        struct barorderNode *prev;
        struct semaphore *bartender_sem;
        //int    STATE; // this order is awaiting fulfillment or not picked up yet
                      // i.e. BUSY/FREE
};

typedef struct barorderNode node;

typedef struct OL {
        node *head;
        node *tail;
} *OrderList;

#endif
