CXX := g++
CXXFLAGS := -std=c++11

SRCS := hw5.cpp
OBJS := $(SRCS:.cpp=.o)
TARGET := program

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
