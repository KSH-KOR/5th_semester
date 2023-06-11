#include <iostream>
#include <fstream>
#include <string>
#include <queue>
#include <vector>
#include <queue>


using namespace std;

struct Header{
    char * city;
    vector<Header *> adjacent_cities;

    static int cmp(Header *left, Header *right)
    {
        return (left->city)[0] < (right->city)[0];
    }

    void sort_adjacent_nodes()
    {
        sort(adjacent_cities.begin(), adjacent_cities.end(), cmp);
    }
};

struct Edge{
    int weight;
    char* city1;
    char* city2;
};


typedef Edge* EdgePtr;
typedef Header* HeaderPtr;

struct CompareWeights {
    bool operator()(EdgePtr edge1, EdgePtr edge2) {
        return edge1->weight > edge2->weight;
    }
};

vector<EdgePtr> edge_list;
vector<HeaderPtr> adjacency_list;
priority_queue<EdgePtr, vector<EdgePtr>, CompareWeights> pq;

EdgePtr create_new_edge(int weight, char * city1, char * city2){
    EdgePtr newEdge = (EdgePtr)malloc(sizeof(Edge)); // allocation pair #3
    newEdge->weight = weight;
    newEdge->city1 = city1;
    newEdge->city2 = city2;
    return newEdge;
}

HeaderPtr find_header(char * city_name){
    for(auto hp: adjacency_list){
        if(!strcmp(hp->city, city_name)) return hp;
    }
    return 0x0;
}

HeaderPtr create_new_header(char * city_name){
    HeaderPtr newHeader = (HeaderPtr)malloc(sizeof(Header)); // allocation pair #4
    newHeader->city = (char *)malloc(sizeof(char)*strlen(city_name)); // allocation pair #5
    strcpy(newHeader->city, city_name);
    adjacency_list.push_back(newHeader);
    return newHeader;
}

void read_from_file(char *file_path)
{
    ifstream file(file_path); // file open pair #1
    if (!file.is_open()) 
    {
        cout << "Failed to open the file." << endl;
        return;
    }

    string line;
    queue<char*> tokens;
    char * bucket = (char*)malloc(sizeof(char)*10); // allocation pair #5
    int idx = 0;
    int row_count = 0;
    vector<char *> city_names;
    queue<int> city_index_infos;
    int skip = 0;
    while (getline(file, line))
    {
        skip++;
        int co = -1;
        // tokenize a line
        for(char c : line){
            if(isspace(c)){
                if(bucket == 0x0 || idx == 0) continue;
                co++;
                bucket[idx] = '\0';
                if(co != 0 && co < skip){
                    idx = 0;
                    continue;
                }
                
                char * l = (char*)malloc(sizeof(char)*(idx+2)); // allocation pair #6
                strcpy(l, bucket);
                tokens.push(l);
                idx = 0;
                continue;
            }
            bucket[idx++] = c;
        }

        // insert token info into edge list
        char * city_name;
        int weight = 0;
        int count = -1;
        while(!tokens.empty()){
            count++;
            char * t = tokens.front(); //count'th token in queue
            tokens.pop();
            
             
            if (!strcmp(t, (char *)"INF")) {
                free(t); // allocation pair #6
                continue;
            }
            try {
                weight = stoi(t);
            } catch (const exception& e) {
                // city name or INF
                
                city_name = (char *)malloc(sizeof(char)*strlen(t));
                strcpy(city_name, t);
                city_names.push_back(city_name);
                free(t); // allocation pair #6
                count = city_names.size() - 2;
                continue;
            }
            edge_list.push_back(create_new_edge(weight, city_name, 0x0));
            city_index_infos.push(count);
            weight = 0;
            free(t); // allocation pair #6
        }
    }
    free(bucket); // allocation pair #5

    file.close(); // file open pair #1
    
    int i;
    for(auto edge : edge_list){
        i = city_index_infos.front();
        city_index_infos.pop();
        edge->city2 = city_names.at(i);
    }
}

void create_adjacency_list(){
    HeaderPtr h;
    HeaderPtr found;
    HeaderPtr found2;
    for(auto edge : edge_list){
        if(edge->weight == 0) continue;
        found = find_header(edge->city1);
        found2 = find_header(edge->city2);
        if(found2 == 0x0) found2 = create_new_header(edge->city2);
        if(found == 0x0) found = create_new_header(edge->city1);

        found->adjacent_cities.push_back(found2);
        found2->adjacent_cities.push_back(found);
    }
}

void print_adjacency_list(){
    printf("-------------adjacency list-------------\n");
    for(auto h : adjacency_list){
        printf("%10s | ", h->city);
        for(auto v : h->adjacent_cities){
            printf("%10s : ", v->city);
        }
        printf("\n");
    }
    printf("----------------------------------------\n");
}

void print_edge_list(){
    printf("-------------edge list-------------\n");
    for(auto c : edge_list){
        printf("%8s --  %3d -- %s\n", c->city1, c->weight, c->city2);
    }
    printf("----------------------------------------\n");
}

void build_priority_queue() {
    for (EdgePtr edge : edge_list) { 
        EdgePtr edge_copy = (EdgePtr)malloc(sizeof(Edge)); // allocation pair #7
        edge_copy->city1 = (char *)malloc(sizeof(char) * (strlen(edge->city1) + 1)); // allocation pair #1
        edge_copy->city2 = (char *)malloc(sizeof(char) * (strlen(edge->city2) + 1)); // allocation pair #2
        strcpy(edge_copy->city1, edge->city1);
        strcpy(edge_copy->city2, edge->city2);
        edge_copy->weight = edge->weight;

        // Inserting the copied edge into the priority queue
        pq.push(edge_copy);
    }

    // Accessing the edges in the priority queue
    while (!pq.empty()) {
        EdgePtr edge = pq.top();
        pq.pop();

        cout << "Weight: " << edge->weight << ", City1: " << edge->city1 << ", City2: " << edge->city2 << endl;

        free(edge->city1); // allocation pair #1
        free(edge->city2); // allocation pair #2
    }
}

void free_global_res(){
    for(auto e: edge_list){
        free(e);
    }
    for(auto a: adjacency_list){
        if(a->city != NULL) free(a->city);
        if(a != NULL) free(a);
    }
}

void Floyd(){

}

int main(void){
    read_from_file((char*)"hw6.txt");
    create_adjacency_list();


    print_edge_list();
    print_adjacency_list();

    build_priority_queue();

    free_global_res();

    return 0;
}