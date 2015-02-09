INCLUDES = -F/System/Library/Frameworks -framework OpenGL -framework GLUT -lm
HEADERS = perlin.h graphics.h
C_FILES = a1.c graphics.c visible.c perlin.c
TARGET = a1

all: default

default: $(TARGET)

$(TARGET): $(C_FILES) $(HEADERS)
	gcc $(C_FILES) -o $(TARGET) $(INCLUDES)
	
clean:
	rm -f *~
	rm -f *.o
	rm -f $(TARGET)

