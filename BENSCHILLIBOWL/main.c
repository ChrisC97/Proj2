#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>

#include "BENSCHILLIBOWL.h"

// Feel free to play with these numbers! This is a great way to
// test your implementation.
#define BENSCHILLIBOWL_SIZE 100
#define NUM_CUSTOMERS 90
#define NUM_COOKS 7
#define ORDERS_PER_CUSTOMER 12
#define EXPECTED_NUM_ORDERS NUM_CUSTOMERS * ORDERS_PER_CUSTOMER

// Global variable for the restaurant.
BENSCHILLIBOWL *bcb;

/**
 * Thread funtion that represents a customer. A customer should:
 *  - allocate space (memory) for an order.
 *  - select a menu item.
 *  - populate the order with their menu item and their customer ID.
 *  - add their order to the restaurant.
 */
void* BENSCHILLIBOWLCustomer(void* tid) {
  int customer_id = (int)(long) tid;
  
  for(int i = 0; i < ORDERS_PER_CUSTOMER; i++){
    // Allocate space for an order.
    Order* myOrder = (Order*) malloc(sizeof(Order));
    // Select a menu item.
    myOrder->menu_item = PickRandomMenuItem();
    myOrder->customer_id = customer_id;
    // Add order to the resaurant.
    AddOrder(bcb, myOrder);
  }
  return NULL;
}

/**
 * Thread function that represents a cook in the restaurant. A cook should:
 *  - get an order from the restaurant.
 *  - if the order is valid, it should fulfill the order, and then
 *    free the space taken by the order.
 * The cook should take orders from the restaurants until it does not
 * receive an order.
 */
void* BENSCHILLIBOWLCook(void* tid) {
  int cook_id = (int)(long) tid;
	int orders_fulfilled = 0;
  Order* lastOrder = NULL;
  // Distribute expected orders across all cooks.
  for(int i = 0; i < ceil((double)EXPECTED_NUM_ORDERS/(double)NUM_COOKS); i++){
    lastOrder = GetOrder(bcb);
    if(lastOrder != NULL){
      orders_fulfilled++;
    }
  }
	printf("Cook #%d fulfilled %d orders\n", cook_id, orders_fulfilled);
	return NULL;
}

/**
 * Runs when the program begins executing. This program should:
 *  - open the restaurant
 *  - create customers and cooks
 *  - wait for all customers and cooks to be done
 *  - close the restaurant.
 */
int main() {
	// Open restaurant.
	bcb = OpenRestaurant(BENSCHILLIBOWL_SIZE, EXPECTED_NUM_ORDERS);
	
	// Create an thread per customer.
	pthread_t tid[NUM_CUSTOMERS];
	// Create an thread per cook.
	pthread_t tid_cooks[NUM_COOKS];
	
	// Create customer threads.
	for(int i = 0; i < NUM_CUSTOMERS; i++){
    pthread_create(&tid[i], NULL, BENSCHILLIBOWLCustomer, (void *)&tid[i]);
	}
	// Create the cooks.
	for(int g = 0; g < NUM_COOKS; g++){
		pthread_create(&tid_cooks[g], NULL, BENSCHILLIBOWLCook, (void *)&tid_cooks[g]);
	}
	int w = 0;
	/// Wait for all customers to be done.
	for (w = 0; w < NUM_CUSTOMERS; w++) {
		pthread_join(tid[w], NULL);
	}
	/// Wait for all cooks to be done.
	for (w = 0; w < NUM_COOKS; w++) {
		pthread_join(tid_cooks[w], NULL);
	}
	
	// Close the restaurant.
	CloseRestaurant(bcb);
	
	return 0;
}
