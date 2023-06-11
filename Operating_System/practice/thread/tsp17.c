#include <stdio.h>
#include <stdlib.h>

int m[17][17] ;

int path[17] ; //global variable -> paths of city
int used[17] ; //bit vector -> wheter the city is already visited
int length = 0 ; //length of the route
int min = -1 ; // minimum length of the routes

void _travel(int idx) { // recursive funciton
	int i ;

	if (idx == 17) {
		length += m[path[16]][path[0]] ; 
		if (min == -1 || min > length) {
			min = length ;

			printf("%d (", length) ;
			for (i = 0 ; i < 17 ; i++) 
				printf("%d ", path[i]) ;
			printf("%d)\n", path[0]) ;	
		}
		length -= m[path[16]][path[0]] ;
	}
	else { 
		for (i = 0 ; i < 17 ; i++) { 
			if (used[i] == 0) { // select a city that has not visited
				path[idx] = i ; 
				used[i] = 1 ;
				length += m[path[idx-1]][i] ; // add length of the previous city and current city;
				_travel(idx+1) ; // if it get returns then, all cities are visited
				length -= m[path[idx-1]][i] ; // restore the length
				used[i] = 0 ; // turn off the used bit
			}
		}
	}
}

void travel(int start) {
	path[0] = start ; // put starting city in the list
	used[start] = 1 ; // starting city is now in the list
	_travel(1) ;
	used[start] = 0 ; 
}

int main() {
	int i, j, t ;

	FILE * fp = fopen("gr17.tsp", "r") ;

	for (i = 0 ; i < 17 ; i++) {
		for (j = 0 ; j < 17 ; j++) {
			fscanf(fp, "%d", &t) ;
			m[i][j] = t ;
		}
	}
	fclose(fp) ;


	for (i = 0  ; i < 17 ; i++) 
		travel(i) ; //i is the starting city
}
