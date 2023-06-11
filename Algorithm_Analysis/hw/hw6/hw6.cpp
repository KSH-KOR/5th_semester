// 21900136 hw6
// references list
// https://www.geeksforgeeks.org/floyd-warshall-algorithm-dp-16/
// https://www.geeksforgeeks.org/dijkstras-shortest-path-algorithm-greedy-algo-7/
// 9. ch24.pdf

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <queue>


const int INF = 99999; // Define the value for infinity

using namespace std;

typedef struct Neighbor {
    int city_index;
    int weight;
    int visited=0;
} Neighbor;

typedef Neighbor * NeighborPtr;

struct CompareWeights {
    bool operator()(NeighborPtr node1, NeighborPtr node2) {
        return node1->weight > node2->weight;
    }
};

vector<vector<NeighborPtr> > adjacency_list;
vector<string> city_list;

NeighborPtr create_new_neighbor(int city_index, int weight){
    NeighborPtr new_neighbor = (NeighborPtr)malloc(sizeof(Neighbor)); // allocation pair #1
    new_neighbor->city_index = city_index;
    new_neighbor->weight = weight;
    return new_neighbor;
}

NeighborPtr create_INF_node(int city_index){
    return create_new_neighbor(city_index, INF);
}

void generateAdjacencyList(const string& filename) {
    ifstream input_file(filename);
    if (!input_file) {
        cout << "Failed to open the file." << endl;
        return;
    }
    string line;

    while (getline(input_file, line)) {
        istringstream iss(line);
        string node;
        string city_name = "";
        vector<NeighborPtr> row;

        while (iss >> node) {
            if (node == "INF") {
                if(city_name == "") {
                    perror("city name should be placed at the beginning of lines!");
                    exit(1);
                }
                row.push_back(create_INF_node(-1));
            } else {
                try {
                    int weight = stoi(node);
                    row.push_back(create_new_neighbor(-1, weight));
                } catch (const invalid_argument& _) {
                    city_name = node;
                    city_list.push_back(city_name); // Handle non-numeric values -> city name
                }
            }
        }

        adjacency_list.push_back(row);
    }

    for (int i = 0; i < adjacency_list.size(); ++i) {
        for (int j = 0; j < adjacency_list[i].size(); ++j) {
            adjacency_list[i][j]->city_index = j;
        }
    }

    for (int i = 0; i < adjacency_list.size(); ++i) {
        printf("Adjacent nodes for %9s -> ", city_list.at(i).c_str());
        for (int j = 0; j < adjacency_list[i].size(); ++j) {
            if (adjacency_list[i][j]->weight != INF) {
                printf("(%9s, %3d)", city_list[adjacency_list[i][j]->city_index].c_str(), adjacency_list[i][j]->weight);
            }
        }
        cout << endl;
    }

    input_file.close();
}

void Floyd(){
    int num_cities = adjacency_list.size();

    vector< vector<int> > distances(num_cities, vector<int>(num_cities, INT_MAX));

    for (int i = 0; i < num_cities; ++i) {
        distances[i][i] = 0;
        for (const auto& neighbor : adjacency_list[i]) {
            int neighbor_city = neighbor->city_index;
            int neighbor_weight = neighbor->weight;
            distances[i][neighbor_city] = neighbor_weight;
        }
    }

    for (int k = 0; k < num_cities; ++k) {
        for (int i = 0; i < num_cities; ++i) {
            for (int j = 0; j < num_cities; ++j) {
                if (distances[i][k] != INT_MAX && distances[k][j] != INT_MAX &&
                    distances[i][k] + distances[k][j] < distances[i][j]) {
                    distances[i][j] = distances[i][k] + distances[k][j];
                }
            }
        }
    }

    printf("--------------Floyd-------------\n");

    printf("             ");

    for(int i=0; i<num_cities; i++)
        printf(" %9s | ", city_list[i].c_str());

    cout << endl;

    for (int i = 0; i < num_cities; ++i) {
        printf(" %9s | ", city_list[i].c_str());
        for (int j = 0; j < num_cities; ++j) {
            if (distances[i][j] != INT_MAX) {
                printf(" %9d | ", distances[i][j]);
            } else {
                printf(" %9s | ", (char* )"No path");
            }
        }
        cout << endl;
    }

    printf("--------------------------------\n");

}
void Dijkstra(int source){
    priority_queue< pair<int, int>, vector< pair<int, int> >, greater< pair<int, int> > > pq;

    int num_cities = adjacency_list.size();

    vector<int> distances(num_cities, INT_MAX);
    vector<bool> visited(num_cities, false);
    distances[source] = 0;
    
    pq.push(make_pair(0, source));

     while (!pq.empty()) {
        int curr_city = pq.top().second;
        pq.pop();

        if (visited[curr_city]) {
            continue;
        }

        visited[curr_city] = true;

        for (const auto& neighbor : adjacency_list[curr_city]) {
            int neighbor_city = neighbor->city_index;
            int neighbor_weight = neighbor->weight;

            // Relaxing part!!
            if (!visited[neighbor_city] && distances[curr_city] != INT_MAX &&
                distances[curr_city] + neighbor_weight < distances[neighbor_city]) {
                distances[neighbor_city] = distances[curr_city] + neighbor_weight;

                pq.push(make_pair(distances[neighbor_city], neighbor_city));
            }
        }
    }

    printf(" %9s | ", city_list[source].c_str());

    for (int i = 0; i < num_cities; ++i) {
        printf(" %9d | ", distances[i]);
    }

    cout << endl;

}

void print_Dijkstra(){
    int num_cities = adjacency_list.size();

    printf("------------Dijkstra------------\n");

    printf("             ");

    for(int i=0; i<num_cities; i++)
        printf(" %9s | ", city_list[i].c_str());

    cout << endl;

    for(int i=0; i<num_cities; i++)
        Dijkstra(i);

    printf("--------------------------------\n");
    
}

void free_res(){
    for(auto a : adjacency_list){
        for(auto b : a){
            free(b); // allocation pair #1
        }
    }
}

int main() {
    string filename = "hw6.txt";

    generateAdjacencyList(filename);

    print_Dijkstra();
    Floyd();

    free_res();

    return 0;
}
