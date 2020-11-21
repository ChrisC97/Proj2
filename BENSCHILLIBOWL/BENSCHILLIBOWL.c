#include "BENSCHILLIBOWL.h"

#include <assert.h>
#include <stdlib.h>
#include <time.h>

bool IsEmpty(BENSCHILLIBOWL* bcb);
bool IsFull(BENSCHILLIBOWL* bcb);
void AddOrderToBack(Order **orders, Order *order);

MenuItem BENSCHILLIBOWLMenu[] = { 
    "BensChilli", 
    "BensHalfSmoke", 
    "BensHotDog", 
    "BensChilliCheeseFries", 
    "BensShake",
    "BensHotCakes",
    "BensCake",
    "BensHamburger",
    "BensVeggieBurger",
    "BensOnionRings",
};
int BENSCHILLIBOWLMenuLength = 10;

/* Select a random item from the Menu and return it */
MenuItem PickRandomMenuItem() {
  return BENSCHILLIBOWLMenu[random_int(0, BENSCHILLIBOWLMenuLength)];
}

/* Allocate memory for the Restaurant, then create the mutex and condition variables needed to instantiate the Restaurant */

BENSCHILLIBOWL* OpenRestaurant(int max_size, int expected_num_orders) {
	// Restaurant Init.
	BENSCHILLIBOWL* bcb = (BENSCHILLIBOWL*) malloc(sizeof(BENSCHILLIBOWL));
	bcb->expected_num_orders = expected_num_orders;
	bcb->max_size = max_size;
  bcb->current_size = 0;
  bcb->orders = NULL;
  bcb->orders_handled = 0;
	// Init mutex.
	if (pthread_mutex_init(&(bcb->mutex), NULL) != 0) { 
			printf("\n mutex init has failed\n"); 
			return NULL; 
	} 
  // Init conditions.
  pthread_cond_init(&(bcb->can_add_orders), NULL);
  pthread_cond_init(&(bcb->can_get_orders), NULL);
	
	printf("Restaurant is open!\n");
	return bcb;
}


/* check that the number of orders received is equal to the number handled (ie.fullfilled). Remember to deallocate your resources */

void CloseRestaurant(BENSCHILLIBOWL* bcb) {
	if(bcb->expected_num_orders == bcb->orders_handled){
		printf("Expected number of orders handled!\n");
	}else{
    printf("DID NOT HANDLE EXPECTED NUMBER OF ORDERS!");
  }
  // Cleanup.
  pthread_mutex_destroy(&bcb->mutex);
  pthread_cond_destroy(&bcb->can_add_orders);
  pthread_cond_destroy(&bcb->can_get_orders);
	pthread_mutex_destroy(&(bcb->mutex));
  free(bcb);
	printf("Restaurant is closed!\n");
}

/* add an order to the back of queue */
int AddOrder(BENSCHILLIBOWL* bcb, Order* order) {
	pthread_mutex_lock(&(bcb->mutex));
	
	// restaurant can't take any orders atm, wait until it can.
	while(IsFull(bcb)){
		pthread_cond_wait(&(bcb->can_add_orders), &(bcb->mutex));
	}
	
  // Assign order it's number.
  order->order_number = bcb->next_order_number;
  bcb->next_order_number++;
  
	// Add order to back of queue.
	AddOrderToBack(&(bcb->orders), order);
  bcb->current_size++;
	pthread_cond_signal(&(bcb->can_get_orders));
	
	pthread_mutex_unlock(&(bcb->mutex)); 
	return (bcb->next_order_number-1);
}

/* remove an order from the queue */
Order *GetOrder(BENSCHILLIBOWL* bcb) {
  // Check if all possible orders have already been handled.
  if (bcb->orders_handled == bcb->expected_num_orders) {
    return NULL;
  }
  
	pthread_mutex_lock(&(bcb->mutex));
	
	// No orders to take and we're still waiting on more.
	while(IsEmpty(bcb)){
		pthread_cond_wait(&(bcb->can_get_orders), &(bcb->mutex));
	}
	
  if(IsEmpty(bcb)){
    return NULL;
  }
  
	// Remove order from the front of queue.
  Order *temp = bcb->orders;
  bcb->orders = temp->next;
	
	// Update restaurant variables.
	bcb->current_size--;
	bcb->orders_handled++;
  pthread_cond_signal(&(bcb->can_add_orders));
	pthread_mutex_unlock(&(bcb->mutex)); 
	return temp;
}

// Optional helper functions (you can implement if you think they would be useful)
bool IsEmpty(BENSCHILLIBOWL* bcb) {
	if(bcb->current_size <= 0){
		return true;
	}
  return false;
}

bool IsFull(BENSCHILLIBOWL* bcb) {
	if(bcb->current_size == bcb->max_size){
		return true;
	}
  return false;
}

/* this methods adds order to rear of queue */
void AddOrderToBack(Order **orders, Order *order) {
  if (!orders[0]) {
        orders[0] = order;
        order->next = NULL;
    }
    Order *temp = orders[0]; 
    while (temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = order;
    order->next = NULL;
    return;
}

// https://stackoverflow.com/questions/29381843/generate-random-number-in-range-min-max
int random_int(int min, int max)
{
   return min + rand() % (max+1 - min);
}
