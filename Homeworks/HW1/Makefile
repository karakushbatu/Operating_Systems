CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = scheduler

all: $(TARGET)

$(TARGET): priority_scheduler.c
	$(CC) $(CFLAGS) -o $(TARGET) priority_scheduler.c

clean:
	rm -f $(TARGET) scheduler.log
