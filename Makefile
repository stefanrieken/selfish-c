CC=gcc
CFLAGS=-fdata-sections -ffunction-sections -Wall -Os -Wl,--gc-sections -fPIC
SOURCES = runtime.c stack.c
OBJECTS = $(SOURCES:.c=.o)
TEST_SOURCES = $(wildcard *.c)
TEST_OBJECTS = $(TEST_SOURCES:.c=.o)

all: test

clean:
	rm -f *.o test

test: $(TEST_OBJECTS)
	$(CC) $(CFLAGS) $(TEST_OBJECTS) -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@
