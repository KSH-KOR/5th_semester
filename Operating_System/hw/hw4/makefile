CC = gcc
CFLAGS = -Wall -Wextra -std=c99

SRCS = findeq.c
OBJS = $(SRCS:.c=.o)
EXECUTABLE = findeq

.PHONY: all clean

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXECUTABLE)

