# Makefile for market_sim

CC       := gcc
CFLAGS   := -std=c11 -Wall -Wextra -pthread
TARGET   := market_sim
SRC      := 150210040_market_sim.c
LOG      := log.txt

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $<

run: $(TARGET)
	@rm -f $(LOG)
	@./$(TARGET)

clean:
	@rm -f $(TARGET) $(LOG)

