CC = gcc
CFLAGS = -Wall
LDFLAGS = -lpthread
OBJFILES = p1.o physical.o
TARGET = p1
all: $(TARGET)

$(TARGET): $(OBJFILES)
        $(CC) $(CFLAGS) -o $(TARGET) -g $(OBJFILES) $(LDFLAGS)
clean:
        rm -f $(OBJFILES) $(TARGET) *~