CC := cc

CPPFLAGS := -Iinclude
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -g

TARGET := bin/venu
VALUE_TEST_TARGET := bin/test_value

SOURCES := \
	src/main.c \
	src/allocator.c \
	src/arena.c \
	src/lexer.c \
	src/parser.c \
	src/runtime.c \
	src/value.c \
	platform/host/platform.c

OBJECTS := $(patsubst %.c,obj/%.o,$(SOURCES))

VALUE_TEST_OBJECTS := \
	obj/tests/test_value.o \
	obj/src/allocator.o \
	obj/src/value.o \
	obj/platform/host/platform.o

.PHONY: all clean run test-value

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

obj/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

test-value: $(VALUE_TEST_TARGET)
	./$(VALUE_TEST_TARGET)

$(VALUE_TEST_TARGET): $(VALUE_TEST_OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(VALUE_TEST_OBJECTS) -o $@

clean:
	rm -rf bin obj