CC := cc

CPPFLAGS := -Iinclude
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -g

TARGET := bin/venu

SOURCES := \
	src/main.c \
	src/arena.c \
	src/lexer.c \
	src/parser.c \
	src/runtime.c \
	platform/host/platform.c

OBJECTS := $(patsubst %.c,obj/%.o,$(SOURCES))

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

obj/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf bin obj