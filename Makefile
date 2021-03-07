CC = /usr/bin/gcc
CFLAGS ?= -g -Wall
INCLUDE = -I./include
SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)
BIN = bfck

all: $(BIN)

%.d: %.c
	$(CC) -MM $< $(CFLAGS) $(INCLUDE) > $@

-include $(OBJS:.o=.d)

%.o: %.c
	$(CC) -c -o $@ $< -MMD $(INCLUDE) $(CFLAGS)

$(BIN): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

test: $(BIN)
	cd tests && make

clean:
	rm -rf $(BIN) $(OBJS)
