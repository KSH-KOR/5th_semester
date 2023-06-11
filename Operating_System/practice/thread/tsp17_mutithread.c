#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int m[17][17] ;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// if there are global variable all threads will share this, and make a mess
/*
int path[17] ; //global variable -> paths of city
int used[17] ; //bit vector -> wheter the city is already visited
int length = 0 ; //length of the route
*/
int min = -1 ; // minimum length of the routes

void _travel(int idx, int * path, int * used, int * length) { // recursive funciton
	int i ;

	if (idx == 17) { // this logic needs to be mutually exclusively run
		*length += m[path[16]][path[0]] ; 
		if (min == -1 || min > *length) {
			pthread_mutex_lock(&lock) ;
			min = *length ;
			printf("%d (", *length) ;
			for (i = 0 ; i < 17 ; i++) 
				printf("%d ", path[i]) ;
			printf("%d)\n", path[0]) ;	
			pthread_mutex_unlock(&lock) ;
		}
		*length -= m[path[16]][path[0]] ;
	}
	else { 
		for (i = 0 ; i < 17 ; i++) { 
			if (used[i] == 0) { // select a city that has not visited
				path[idx] = i ; 
				used[i] = 1 ;
				*length += m[path[idx-1]][i] ; // add length of the previous city and current city;
				_travel(idx+1, path, used, length) ; // if it get returns then, all cities are visited
				*length -= m[path[idx-1]][i] ; // restore the length
				used[i] = 0 ; // turn off the used bit
			}
		}
	}
}

void* travel(void* arg) {
	int path[17] ; //global variable -> paths of city
	int used[17] ; //bit vector -> wheter the city is already visited
	int length = 0 ; //length of the route
	int start = *((int *)arg);
	free(arg);
	path[0] = start ; // put starting city in the list
	used[start] = 1 ; // starting city is now in the list
	_travel(1, path, used, &length) ;
	used[start] = 0 ; 
	return 0x0;
}

int main() {
	int i, j, t ;
	pthread_t threads[17];

	FILE * fp = fopen("gr17.tsp", "r") ;

	for (i = 0 ; i < 17 ; i++) {
		for (j = 0 ; j < 17 ; j++) {
			fscanf(fp, "%d", &t) ;
			m[i][j] = t ;
		}
	}
	fclose(fp) ;

	// pthread_mutex_init(&lock, NULL);
	for (i = 0  ; i < 17 ; i++) {
		int * start = (int*)malloc(sizeof(int));
		pthread_create(&(threads[i]),
                          NULL,
                          travel,
                          (void * )start); // you need to pass pointer because it has to be in heap memory.
	}

	// if you code like this then there is no ordering between 64 pthread_create and travel function. so all thread amy not get expected variable 0 to 16
	// lock cannot specify the ordering between threads. it is only for exclusive muture
	/*
	for (i = 0  ; i < 17 ; i++) {
		pthread_create(threads[i],
                          NULL,
                          travel,
                          (void * )&i); // you need to pass pointer because it has to be in heap memory.
		// travel(i) ; //i is the starting city
	}
	*/

	// if you dont join, when the main thread is termianted the other threads get just terminated
	for (i = 0  ; i < 17 ; i++) {
		pthread_join(threads[i], NULL);
	}
		
}
