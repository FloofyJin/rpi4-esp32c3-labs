EXEC	= send

SOURCES	= send.c
OBJECTS	= $(SOURCES:%.c=%.o)

CC	= gcc
LFLAGS	= -lgpiod

.PHONY: all format clean

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) -o $@ $^ $(LFLAGS)

%.o: %.c
	$(CC) -c $<

format:
	clang-format -i -style=file *.[ch]

clean:
	rm -rf $(EXEC) $(OBJECTS)
