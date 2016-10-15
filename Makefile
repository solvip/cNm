DAEMON_TARGET = server
CLIENT_TARGET = client
LIBS = -lev
CC = cc
CFLAGS = -std=c11 -g -Wall -Wpedantic -Wextra -Wstrict-overflow -fno-strict-aliasing -march=native

.PHONY: all clean test

all: $(DAEMON_TARGET) $(CLIENT_TARGET) test

OBJECTS = $(patsubst %.c, %.o, $(wildcard src/*.c))
HEADERS = $(wildcard *.h)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

$(DAEMON_TARGET): $(OBJECTS)
	$(CC) $(filter-out src/$(CLIENT_TARGET).o, $(OBJECTS)) $(CFLAGS) $(LIBS) -o $@

$(CLIENT_TARGET): $(OBJECTS)
	$(CC) $(filter-out src/$(DAEMON_TARGET).o, $(OBJECTS)) $(CFLAGS) $(LIBS) -o $@

# Compile the test sources.
TEST_SOURCES = $(wildcard tests/*.c)
TEST_OBJECTS = $(filter-out src/$(DAEMON_TARGET).o src/$(CLIENT_TARGET).o, $(OBJECTS))
test: $(OBJECTS) $(TEST_SOURCES)
	$(CC) $(CFLAGS) -I src/ -Wall $(LIBS) -lcriterion $(TEST_OBJECTS) $(TEST_SOURCES) -o $@
	./test

clean:
	-rm -f $(OBJECTS)
	-rm -f $(TARGET) test
