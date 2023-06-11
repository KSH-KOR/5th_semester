/*
21900136 김신후 hw5

reference:
1. https://www.programiz.com/cpp-programming/library-function/cctype/isspace
2. dfs_2.cpp
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stack>

using namespace std;

enum COLOR
{
    WHITE,
    GRAY,
    BLACK
};

struct Header
{
    char title;
    COLOR color;
    vector<Header *> adjacent_nodes;
    int discover;
    int finish;

    Header(char title)
    {
        this->color = WHITE;
        this->title = title;
    }
    Header(Header *from)
    {
        this->color = from->color;
        this->title = from->title;
        this->discover = from->discover;
        this->finish = from->finish;
    }

    static int cmp(Header *left, Header *right)
    {
        return left->title < right->title;
    }

    void sort_adjacent_nodes()
    {
        sort(adjacent_nodes.begin(), adjacent_nodes.end(), cmp);
    }

    void set_discovered(int *times)
    {
        discover = *times;
        color = GRAY;
    }
    void set_finished(int *times)
    {
        finish = *times;
        color = BLACK;
    }
    bool has_not_discovered()
    {
        return color == WHITE;
    }
};

void increment(int *i)
{
    (*i) = (*i) + 1;
}
void decrement(int *i)
{
    (*i) = (*i) - 1;
}

typedef Header *HeaderPtr;

// global variable
vector<HeaderPtr> header_list;
vector<HeaderPtr> transposed_header_list;

HeaderPtr get_new_header(char title)
{
    HeaderPtr new_header = (HeaderPtr)malloc(sizeof(Header));
    new_header->color = WHITE;
    new_header->title = title;
    return new_header;
}
HeaderPtr find_header(vector<HeaderPtr> list, char title)
{
    if (list.size() == 0)
        return 0x0;

    for (HeaderPtr n : list)
    {
        if (n->title == title)
            return n;
    }
    return 0x0;
}
HeaderPtr find_header(vector<HeaderPtr> list, int times)
{
    if (list.size() == 0)
        return 0x0;

    for (HeaderPtr n : list)
    {
        if (n->finish == times || n->discover == times)
            return n;
    }
    return 0x0;
}
void read_from_file(char *file_path)
{
    ifstream file(file_path);
    if (!file.is_open())
    {
        cout << "Failed to open the file." << endl;
        return;
    }

    string line;
    while (getline(file, line))
    {

        if (header_list.size() == 0)
        { // initialize header
            for (char c : line)
            {
                if (isspace(c) != 0)
                    continue; // to deal whitespace between data
                else
                {
                    HeaderPtr t = (HeaderPtr)malloc(sizeof(Header));
                    t->title = c;
                    header_list.push_back(t);
                }
            }
            continue;
        }

        char title = line[0];
        HeaderPtr found_header = find_header(header_list, title);
        if (found_header == 0x0)
        {
            cout << "invalid header: " << title << endl;
            continue;
        }

        unsigned long j = 0;
        for (int i = 1; i < line.length(); i++)
        {
            if (line[i] == ' ')
                continue;
            if (line[i] == '1')
            {
                HeaderPtr t = (header_list.at(j++));
                (found_header->adjacent_nodes).push_back(t);
            }
            else
            {
                j++;
            }
        }
    }

    file.close();

    for (HeaderPtr n : header_list)
    {
        n->sort_adjacent_nodes();
    }
}

void print_adjacent_list(vector<HeaderPtr> list)
{
    printf("--adjacent list--\n");
    for (HeaderPtr h : list)
    {
        cout << h->title << " | ";
        for (HeaderPtr n : h->adjacent_nodes)
        {
            cout << n->title << " : ";
        }
        cout << endl;
    }
    printf("-----------------\n");
}

void print_graph(vector<HeaderPtr> list)
{
    for (HeaderPtr h : list)
    {
        printf("node: %c -> discovered time: %2d | finished time: %2d \n", h->title, h->discover, h->finish);
    }
}

int num_nodes()
{
    return header_list.size();
}

void DFS_travel(HeaderPtr start, int *times)
{
    increment(times);
    start->set_discovered(times);

    for (HeaderPtr next : start->adjacent_nodes)
    {
        if (next->has_not_discovered())
        {
            DFS_travel(next, times);
        }
    }
    increment(times);
    start->set_finished(times);
}
void DFS()
{
    int *times = (int *)malloc(sizeof(int));
    *times = 0;
    for (HeaderPtr h : header_list)
    {
        if (h->has_not_discovered())
        {
            DFS_travel(h, times);
        }
    }
    free(times);
}
void overwrite_header(HeaderPtr to, HeaderPtr from){
    to->color = from->color;
    to->title = from->title;
    to->discover = from->discover;
    to->finish = from->finish;
}
HeaderPtr clone_header(HeaderPtr header){
    HeaderPtr clone = (HeaderPtr)malloc(sizeof(Header));
    overwrite_header(clone, header);
    return clone ;
}
HeaderPtr add_new_header(vector<HeaderPtr>& list, HeaderPtr header){
    HeaderPtr added_header;
    if((added_header = find_header(list, header->title)) != 0x0) return added_header;
    HeaderPtr clone = clone_header(header);
    list.push_back(clone);
    return clone ;
}
int cmpx(HeaderPtr left, HeaderPtr right)
{
    return left->title < right->title;
}
void transpose(vector<HeaderPtr>& list, vector<HeaderPtr>& transpose_list){
    for(auto h : list){
        for(auto a : h->adjacent_nodes){
            auto f = add_new_header(transpose_list, a);
            f->adjacent_nodes.push_back(clone_header(h));
        }
    }
    for (HeaderPtr n : transpose_list)
    {
        n->sort_adjacent_nodes();
    }
    
    sort(transpose_list.begin(), transpose_list.end(), cmpx);
}

int main()
{
    read_from_file((char* )"hw5_data.txt");
    print_adjacent_list(header_list);
    DFS();
    print_graph(header_list);

    printf("transposing...\n");

    transpose(header_list, transposed_header_list);
    print_adjacent_list(transposed_header_list);
    print_graph(transposed_header_list);
    return 0;
}