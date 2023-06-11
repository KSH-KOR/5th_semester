/*
 list all the references when doing this homework.
1. page 14 of lecture slide number 6: a pseudocode about dynamic programming
2. https://cplusplus.com/reference/cstdlib/rand/
3. https://www.geeksforgeeks.org/0-1-knapsack-using-branch-and-bound/
4. clique.cpp (to implement timer)
5. https://kbj96.tistory.com/15
6. https://cplusplus.com/reference/queue/priority_queue/
7. https://www.tutorialspoint.com/cpp_standard_library/cpp_queue_copy_constructor.htm#:~:text=The%20C%2B%2B%20copy%20constructor%20std,present%20in%20existing%20queue%20other.
*/

#include <string>
#include <vector>
#include <queue>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <cstdlib>

using namespace std;

int* B;
int* W;
int capacity;
int n_item;

struct history {
    double time;
    int max_benefit_val;
    int nsize;
    int option;
    history(double t, int m, int n, int o) {
        time = t;
        max_benefit_val = m;
        nsize = n;
        option = o;
    }
};

vector<history> his;

class invalidOptionException : exception {
public:
    const char* what(int option) {
        return "invalid option. try again\n";
    }
};

class invalidNItemException : exception {
public:
    const char* what(int n) {
        return "invalid number of items. try again\n";
    }
};

int** request_2d_array(size_t n, size_t w) {
    int** array = new int* [n];
    for (int i = 0; i < n; i++) {
        array[i] = new int[w];
    }
    return array;
}

int* gen_random_number(int n, pair<int, int> bb) {
    int* bucket;
    bucket = (int*)malloc(sizeof(int) * n);
    for (int i = 0; i < n; i++) {
        bucket[i] = rand() % bb.second + bb.first;
    }
    return bucket;
}

void initialize_items(int n) {
    free(B);
    free(W);

    B = gen_random_number(n, make_pair(1, 500));
    W = gen_random_number(n, make_pair(1, 100));
    capacity = n * 25;
}

// maximum of two integers
int max(int a, int b) { return (a > b) ? a : b; }

int bf(int W, int wt[], int val[], int n)
{
    // Base Case
    if (n == 0 || W == 0)
        return 0;

    if (wt[n - 1] > W)
        return bf(W, wt, val, n - 1);

    else
        return max(
            val[n - 1]
            + bf(W - wt[n - 1], wt, val, n - 1),
            bf(W, wt, val, n - 1));
}

struct Item
{
    int weight;
    int value;

    string toString() {
        return "weight: " + to_string(weight) + " | value: " + to_string(value) + " | rate: " + to_string(value / weight);
    }
};

Item* gen_items() {
    Item* p = (Item*)malloc(sizeof(Item) * n_item);
    for (int i = 0; i < n_item; i++) {
        p[i] = { W[i], B[i] };
    }
    return p;
}

bool cmp(Item a, Item b)
{
    double r1 = (double)a.value / a.weight;
    double r2 = (double)b.value / b.weight;
    return r1 > r2;
}

double greedy(int W, struct Item arr[], int N)
{
    sort(arr, arr + N, cmp);

    double finalvalue = 0.0;

    for (int i = 0; i < N; i++) {

        if (arr[i].weight <= W) {
            W -= arr[i].weight;
            finalvalue += arr[i].value;
        }

        else {
            finalvalue
                += arr[i].value
                * ((double)W / (double)arr[i].weight);
            break;
        }
    }

    return finalvalue;
}

struct Node
{
    int level;
    int value, bound, weight;

    Node() {}

    bool operator<(const Node n) const {
        return this->bound < n.bound;
    }

    string toString() {
        return "level: " + to_string(level) + " | value: " + to_string(value) + " | bound: " + to_string(bound) + " | weight: " + to_string(weight);
    }
};




int bound(Node u, int n, int W, Item arr[])
{
    if (u.weight >= W)
        return 0;

    int profit_bound = u.value;
    int total_weight = u.weight;

    int i = u.level + 1;


    while ((i < n) && (total_weight + arr[i].weight <= W))
    {
        total_weight += arr[i].weight;
        profit_bound += arr[i].value;
        i++;
    }

    // fractional
    int room_weight = W - total_weight;
    if (i < n && room_weight > 0)
        profit_bound += room_weight * ((double)arr[i].value / (double)arr[i].weight);

    return profit_bound;
}

void print_queue(priority_queue<Node> Q) {
    if (Q.empty()) {
        cout << "queue is empty" << endl;
        return;
    }
    priority_queue<Node> q(Q);
    cout << "=====================================================================" << endl;
    while (!q.empty())
    {
        Node n = q.top();
        cout << n.toString() << endl;
        q.pop();
    }
    cout << "=====================================================================" << endl;
    std::cout << std::endl;
}

void print_item_arr(Item arr[], int n) {
    for (int i = 0; i < n; i++) {
        cout << arr[i].toString() << endl;
    }
}

int bb(int W, Item arr[], int n)
{
    int max_benefit = 0;
    sort(arr, arr + n, cmp);
    priority_queue<Node> Q;
    Node u;
    u.value = u.weight = u.level = u.bound =  0;
    Q.push(u);


    while (!Q.empty())
    {
        u = Q.top();
        Q.pop();
        Node left;
        left.level = u.level + 1;
        left.weight = u.weight + arr[u.level].weight;
        left.value = u.value + arr[u.level].value;
        if (left.weight <= W) {
            if (left.value > max_benefit)
                max_benefit = left.value;
            left.bound = bound(left, n, W, arr);
            if (left.bound > max_benefit)
                Q.push(left);

        }

        Node right;
        right.level = u.level + 1;
        right.weight = u.weight;
        right.value = u.value;
        right.bound = bound(right, n, W, arr);
        if (right.bound > max_benefit)
            Q.push(right);

        // print_queue(Q);
        //_sleep(1);
    }

    return max_benefit;
}


int deallocate(int** p, size_t n, size_t w) {
    for (int i = 0; i < n; i++) {
        delete[] p[i];
    }
    delete[] p;
    return 1;
}


int dp(int W, int wt[], int val[], int n)
{
    int i, w;
    int** K = request_2d_array(n + 1, W + 1);

    for (i = 0; i <= n; i++) {
        for (w = 0; w <= W; w++) {
            if (i == 0 || w == 0)
                K[i][w] = 0;
            else if (wt[i - 1] <= w)
                K[i][w] = max(val[i - 1]
                    + K[i - 1][w - wt[i - 1]],
                    K[i - 1][w]);
            else
                K[i][w] = K[i - 1][w];
        }
    }
    int answer = K[n][W];
    deallocate(K, n + 1, W + 1);

    return answer;
}

bool validate_option(int o) {
    if (o >= 0 && o <= 6) {
        return true;
    }
    else {
        throw invalidOptionException();
    }
}
bool validate_nitem(int n) {
    if (n > 0 && n <= 10000) {
        return true;
    }
    else {
        throw invalidNItemException();
    }
}

enum options {
    x, BruteForce, Greedy, DP, BB
};

void execute_option(int option) {

    int max_benefit_val = 0;
    clock_t start = 0, stop = 0;
    double duration = 0;
    start = clock();

    switch (option) {
    case 1:
        printf("Brute force...\n");
        max_benefit_val = bf(capacity, W, B, n_item);
        printf("done!\n");
        break;
    case 2:
        printf("Greedy...\n");
        max_benefit_val = greedy(capacity, gen_items(), n_item);
        printf("done!\n");
        break;
    case 3:
        printf("Dynamic Programming...\n");
        max_benefit_val = dp(capacity, W, B, n_item);
        printf("done!\n");
        break;
    case 4:
        printf("Beanch and Bound...\n");
        max_benefit_val = bb(capacity, gen_items(), n_item);
        printf("done!\n");
        break;
    }

    stop = clock();
    duration = (double)(stop - start);
    his.push_back(history(duration, max_benefit_val, n_item, option));
}

void show_history() {
    printf("option\tN\tDuration(ms)\tMaximum benefit value\n");
    for (auto h : his) {
        printf("[%d]\t%d\t%.3lf\t\t%d\n", h.option, h.nsize, h.time, h.max_benefit_val);
    }
}

int main() {
    srand(time(NULL));
    int option;
    int old = -1;

    while (1) {
        printf("\n----------------------\n");
        printf("select option\n");
        printf("1. Brute force \n2. Greedy \n3. Dynamic Programming \n4. Branch and bound\n-- \n5. all at once \n6. show history \n0. quit \n >> ");
        cin >> option;
        try {
            if (validate_option(option)) {
                printf("selected option: %d\n", option);
            }
        }
        catch (invalidOptionException e) {
            printf("%s", e.what(option));
            continue;
        }
        if (option == 0) break;
        if (option == 6) {
            show_history();
            continue;
        }

        printf("enter number of items\n >>");
        cin >> n_item;
        try {
            if (validate_nitem(n_item)) {
                printf("selected number of items: %d\n", n_item);
            }
        } catch(invalidNItemException e){
            printf("%s", e.what(n_item));
            continue;
        }

        if (old != n_item) {
            initialize_items(n_item);
            old = n_item;
        }

        if (option == 5) {
            for (int i = 2; i <= 4; i++) {
                execute_option(i);
            }
        }
        else {
            execute_option(option);
        }



    }
    show_history();
    cout << "BYE!" << endl;

}