#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>

using namespace std;

typedef struct g_node {
    int city;
	int distance;    
    struct g_node *link;   
} list_node;

typedef struct h_node {
    int data;    
    struct g_node *link;   
} head_node;

enum COLOR {WHITE, GRAY, BLACK};

const int num_nodes = 5;

Create_array(head_node **my_graph){
	*my_graph = new head_node[num_nodes];
	for (int i=0; i<num_nodes; i++){
		(*my_graph)[i].data = i + 1;
		(*my_graph)[i].link = NULL;
	}
}

void Read_from_file(int distance[][num_nodes]){
	ifstream inputFile("data.txt");
    for (int i = 0; i < num_nodes; i++) {
        for (int j = 0; j < num_nodes; j++) {
            inputFile >> distance[i][j];
            cout << distance[i][j] << '\t';
        }
        cout << endl;
    }
    cout << endl;
}
    
void Insert(int data[][num_nodes], head_node *my_graph){
	for (int i = 0; i < num_nodes; i++) {
        for (int j = 0; j < num_nodes; j++) {
        	if (data[i][j] > 0){
        		list_node *new_node = new list_node;
				new_node->city = j;
				new_node->distance = data[i][j];
				new_node->link = NULL;
				
				list_node *temp = my_graph[i].link;
				if (temp==NULL)
					my_graph[i].link = new_node;
				else{
					list_node *prev=NULL;
					while (temp!=NULL){
						prev = temp;
						temp = temp->link;
					}
					prev->link = new_node;
				}
			}
		}
	}
}

void Display_Lists(head_node *my_graph){
	for (int i=0; i<num_nodes; i++){
		list_node *temp = my_graph[i].link;
		cout << "From " << i+1 << ": ";
		while (temp != NULL){
			cout << temp->city << ", " << temp->distance << "     ";
			temp = temp->link;
		}
		cout << endl;
	}
}

void DFS_VISIT(head_node *my_graph, int u, enum COLOR color[], int *time, int d[], int f[]){
	color[u] = GRAY;
	*time += 1;
	d[u] = *time;
	list_node *temp = my_graph[u].link;
	while (temp != NULL){
		if (color[temp->city] == WHITE) DFS_VISIT(my_graph, temp->city, color, time, d, f);
		temp = temp->link;
	}
	color[u] = BLACK;
	*time += 1;
	f[u] = *time;	
}

void DFS(head_node *my_graph, enum COLOR color[], int *time, int d[], int f[]){
    for (int i=0; i<num_nodes; i++){
    	if (color[i] == WHITE){
    		DFS_VISIT(my_graph, i, color, time, d, f);
		}
	}
}

Display_d_f_time(int d[], int f[]){
	cout << "Discovery time: ";
	for (int i=0; i<num_nodes; i++){
		cout << "(" << i << "," << d[i] << ")" << '\t'; 
	}
	cout << endl;
	
	cout << "Finish time: ";
	for (int i=0; i<num_nodes; i++){
		cout << "(" << i << "," << f[i] << ")" << '\t'; 
	}
	cout << endl;
}

int main()
{
    head_node *my_graph=NULL;
	int distance[num_nodes][num_nodes];
	
	// Create array
	Create_array(&my_graph);
	
	// Print array
	for (int i=0; i<num_nodes; i++){
		cout << i << ": ";
		cout << my_graph[i].data << ", ";
		cout << my_graph[i].link << endl;
	}
	cout << endl;
	
	//Read data from file
	Read_from_file(distance);

	// Create list
	Insert(distance, my_graph);
    
    // Display
    Display_Lists(my_graph);
    cout << endl;
	
	enum COLOR V_color[num_nodes];
	int d[num_nodes]={0}, f[num_nodes]={0}; 
	for (int i=0; i<num_nodes+1; i++)	V_color[i] = WHITE;
	int time=0;
	
	//Call DFS
	DFS(my_graph, V_color, &time, d, f);	
	
	// Display results
	Display_d_f_time(d,f);

}

