#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int m[17][17] ;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
// int path[17] ; //global variable -> paths of city
// int used[17] ; //bit vector -> wheter the city is already visited
// int length = 0 ; //length of the route
int min = -1 ; // minimum length of the routes

void _travel(int idx, int * path, int * used, int * length, int * local_min) { // recursive funciton
	int i ;

	if (idx == 17) {
		*length += m[path[16]][path[0]] ; 
		if(*local_min == -1 || *local_min > *length){
			pthread_mutex_lock(&lock);
				*local_min = min;
				if (min == -1 || min > *length) {
					min = *length ;
					printf("%d (", *length) ;
					for (i = 0 ; i < 17 ; i++) 
						printf("%d ", path[i]) ;
					printf("%d)\n", path[0]) ;	
				}
			pthread_mutex_unlock(&lock);
		}
		
		*length -= m[path[16]][path[0]] ;
	}
	else { 
		for (i = 0 ; i < 17 ; i++) { 
			if (used[i] == 0) { // select a city that has not visited
				path[idx] = i ; 
				used[i] = 1 ; 
				*length += m[path[idx-1]][i] ; // add length of the previous city and current city;
				_travel(idx+1, path, used, length, local_min) ; // if it get returns then, all cities are visited
				*length -= m[path[idx-1]][i] ; // restore the length
				used[i] = 0 ; // turn off the used bit
			}
		}
	}
}

void* travel(void *ptr) {
	int path[17] ; //local variable -> paths of city
	int used[17] ; //bit vector -> wheter the city is already visited
	int length = 0 ; //length of the route
	int start = *((int*)ptr);
	int local_min = -1;
	free(ptr);
	
	path[0] = start ; // put starting city in the list
	used[start] = 1 ; // starting city is now in the list
	_travel(1, path, used, &length, &local_min) ;
	used[start] = 0 ; 

	return NULL;
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

	for (i = 0  ; i < 17 ; i++){
		int* start = (int*) malloc(sizeof(int));
		pthread_create(&(threads[i]), NULL, travel, (void*) start);
	}

	for (i = 0; i < 17; i++) {
		pthread_join(threads[i], NULL);
	}
}
