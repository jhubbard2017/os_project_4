/*

Project 4

Owner: Jordan Hubbard
Date: November 30, 2017
Instructor: Dr. Hong

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>

// Constants
#define NUMBER_OF_CUSTOMERS 5
#define NUMBER_OF_RESOURCES 3

// Mutexes
pthread_mutex_t mAvailable;
pthread_mutex_t mAllocation;
pthread_mutex_t mMaximum;
pthread_mutex_t mNeed;

// Matrices for banker
int available[NUMBER_OF_RESOURCES];
int maximum[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
int allocation[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
int need[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];

// Methods
void show_matrix(int array[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES], char *array_name);
int request_resources(int customer, int resources[]);
int release_resources(int customer, int resources[]);
int check_if_safe();


void init_maximum() {
	/* Method to initialize the maximum matrix with random values*/
	srand(time(NULL));
	for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
		for (int j = 0; i < NUMBER_OF_RESOURCES; i++) {
			maximum[i][j] = rand() % available[j];
			allocation[i][j] = 0;
		}
	}
}

int release(int customer, int resources[], int safe) {
	/* Method to release resources, updating available and needed resources for customer

	args:
		customer: int
		resources: [int]

	returns:
		int
	 */
	for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
		// Update resources allocated for customer
		pthread_mutex_lock(&mAllocation);
		allocation[customer][i] -= resources[i];
		pthread_mutex_unlock(&mAllocation);

		// Update resources available
		pthread_mutex_lock(&mAvailable);
		available[i] += resources[i];
		pthread_mutex_unlock(&mAvailable);

		// Update resources needed for customer
		pthread_mutex_lock(&mNeed);
		need[customer][i] += maximum[customer][i] + allocation[customer][i];
		pthread_mutex_unlock(&mNeed);
	}
	if (safe < 0) {
		return 1;
	}
	return 0;
}

int request(int customer, int resources[]) {
	/* Method to allocate resources, updating avilable and needed resources for customer

	args:
		customer: int
		resources: [int]

	returns:
		int
	 */
	for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
		// Update resources allocated for customer
		pthread_mutex_lock(&mAllocation);
		allocation[customer][i] += resources[i];
		pthread_mutex_unlock(&mAllocation);

		// Update resources available
		pthread_mutex_lock(&mAvailable);
		available[i] -= resources[i];
		pthread_mutex_unlock(&mAvailable);

		// Update resources needed for customer
		pthread_mutex_lock(&mNeed);
		need[customer][i] -= resources[i];
		pthread_mutex_unlock(&mNeed);
	}

	int safe = check_if_safe();
	if (safe < 0) {
		return release(customer, resources, safe);
	}
	return 0;
}

int request_resources(int customer, int resources[]) {
	/* Method to request resources for a customer 

	args:
		customer: int
		request: [int]

	returns:
		int
	*/
	printf("\n Recieved request from customer: %d", customer);
	for (int i = 0; i < NUMBER_OF_RESOURCES; i++) printf("%d, ", resources[i]);
	printf("\n");

	for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
		if (resources[i] > need[customer][i]) return 1;
	}

	printf("\n Resources available:\n");
	for (int i = 0; i < NUMBER_OF_RESOURCES; i++) printf("%d, ", available[i]);
	printf("\n");

	show_matrix(allocation, "Allocation");
	show_matrix(need, "Need");
	show_matrix(maximum, "Maximum");

	return request(customer, resources);
}

int release_resources(int customer, int resources[]) {
	/* Method for releasing resources used by customer
	
	args:
		customer: int
		release: [int]

	returns:
		int
	*/
	return release(customer, resources, 0);
}

int check_if_safe() {
	/* Method to check if request allocation is safe to execute

	returns:
		int
	 */
	int work[NUMBER_OF_RESOURCES], finish[NUMBER_OF_CUSTOMERS], success = 0;

	// Initialize work and finish
	for (int i = 0; i < NUMBER_OF_RESOURCES; i++) work[i] = available[i];
	for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) finish[i] = 0;

	// Check using banker's algorithm
	for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
		if (finish[i] == 0) {
			for (int j = 0; i < NUMBER_OF_RESOURCES; i++) {
				if (need[i][j] > work[j]) return -1;
			}
			for (int j = 0; j < NUMBER_OF_RESOURCES; j++) work[j] += allocation[i][j];
			success = 1;
		}
	}
	return success;
}

void *create_thread(void *customer) {
	/* Method to create a thread for a customer

	args:
		customer: void*
	*/
	int request[NUMBER_OF_RESOURCES], flag = 0;
	int customer_number = (int)customer;

	for (int i = 0; i < NUMBER_OF_RESOURCES; i++) request[i] = rand() % available[i];

	if (request_resources(customer_number, request) < 0) {
		printf("%d Customer Request Denied.\n", customer_number);
	} else {
		flag = 1;
		printf("%d Customer Request Accepted.\n", customer_number);
	}
	printf("\nRequest:\n");
	for (int i = 0; i < NUMBER_OF_RESOURCES; i++) printf("%d, ", request[i]);
	printf("\n");

	if (flag == 1) {
		sleep(rand() % 10);
		release_resources(customer_number, request);
		printf("\n Released resources from customer: %d", customer_number);
	}
}

void show_matrix(int matrix[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES], char *name) {
	/* Method to print out matrix

	args:
		matrix: array[int][int]
		name: char*
	*/
	printf("Matrix: %s\n", name);
	for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
		for (int j = 0; j < NUMBER_OF_RESOURCES; j++) printf("%d, ", matrix[i][j]);
		printf("\n");
	}
}

int main(int argc, const char * argv[]) {
	/* Main function */
	int customer_count = 0;
	pthread_t tid;

	// Get number of customers
	printf("\nNumber of Customers: ");
	scanf("%d", &customer_count);

	if ((NUMBER_OF_RESOURCES + 1) == argc) {
		printf("\n Resources available:\n");
		for (int i = 0; i < NUMBER_OF_RESOURCES; i++) { 
			available[i] = atoi(argv[i]);
			printf("%d, ", available[i]);	
		}
		printf("\n");
	}

	init_maximum();
	for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
		for (int j = 0; j < NUMBER_OF_RESOURCES; j++) need[i][j] = maximum[i][j] - allocation[i][j];
	}
	show_matrix(maximum, "Maximum");
	show_matrix(need, "Need");

	for (int i = 0; i < customer_count; i++) {
		printf("\n Creating customer with id: %d", i);
		pthread_create(&tid, NULL, create_thread, (void *)i);
	}

	printf("\nExiting Program\n");
	return 0;
}



