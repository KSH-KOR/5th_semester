#include <iostream>
#include <chrono>

using namespace std;
using namespace chrono;

int fibRec(int n) {
    if (n <= 1)
        return n;
    return fibRec(n - 1) + fibRec(n - 2);
}

int fibMas(int n) {
    int fib[n+1];
    fib[0] = 0;
    fib[1] = 1;
    for (int i = 2; i <= n; i++)
        fib[i] = fib[i-1] + fib[i-2];
    return fib[n];
}

int main() {
    int n ;
    cout << "enter n" << endl;
    cin >> n;
    auto start = high_resolution_clock::now();
    cout << "Fibonacci number at position " << n << ": " << fibRec(n) << endl;
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    cout << "Time taken by recursive solution: " << duration.count() << "ms" << endl;

    start = high_resolution_clock::now();
    cout << "Fibonacci number at position " << n << ": " << fibMas(n) << endl;
    end = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(end - start);
    cout << "Time taken by dynamic programming solution: " << duration.count() << "ms" << endl;
    
    return 0;
}
