CXX := g++
CXXFLAGS := -std=c++11

SRCS := hw6.cpp
OBJS := $(SRCS:.cpp=.o)
TARGET := shortestPath

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
