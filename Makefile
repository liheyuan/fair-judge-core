CC = gcc
CFLAGS = -Wall -g -O2 -pthread
LDFLAGS = -lpthread
TARGET = fail-judge-core
SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f $(TARGET) $(OBJS)
