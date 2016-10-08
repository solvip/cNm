TARGET = c1m
LIBS = -lev
CC = cc
CFLAGS = -std=c11 -g -Wall -Wextra -Wpedantic -Wstrict-overflow -fno-strict-aliasing -march=native

.PHONY: all clean test

all: $(TARGET)

OBJECTS = $(patsubst %.c, %.o, $(wildcard src/*.c))
HEADERS = $(wildcard *.h)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS) test
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

# Compile the test sources.
# We exluce c1m.o, as that contains the main function.  We want criterion to define main.
TEST_SOURCES = $(wildcard tests/*.c)
TEST_OBJECTS = $(filter-out src/c1m.o, $(OBJECTS))
test: $(OBJECTS) $(TEST_SOURCES)
	$(CC) $(CFLAGS) -Wall $(LIBS) -lcriterion $(TEST_OBJECTS) $(TEST_SOURCES) -o $@
	./test

clean:
	-rm -f $(OBJECTS)
	-rm -f $(TARGET) test
