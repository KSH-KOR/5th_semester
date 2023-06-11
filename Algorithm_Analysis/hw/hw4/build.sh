#!/bin/bash

if [[ $(uname) == "Darwin" || $(uname) == "Linux" ]]; then
    # for macOS or Linux
    g++ knapsack.cpp -o a.out -std=c++11
    ./a.out
elif [[ $(uname) == "MINGW"* ]]; then
    # for Windows using MinGW or Git Bash
    g++ knapsack.cpp -o a.exe -std=c++11
    ./a.exe
else
    echo "Unsupported operating system."
fi