TARGET = sand
LIBS = -lraylib
CC = g++
CFLAGS = -g -O0 -Wall

.PHONY: clean all default

default: $(TARGET)
all: default

$(TARGET):
	$(CC) $(CFLAGS) $(LIBS) main.cpp -o $@

clean:
	-rm $(TARGET)
