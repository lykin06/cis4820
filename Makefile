TARGET = a1
LIBS = -lm
CC = gcc
CFLAGS = -g -Wall
OPENGL = -lglut -lGLU -lGL

.PHONY: default all clean

default: $(TARGET)
all: default

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) $(OPENGL) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)
	-rm -f *~
	
