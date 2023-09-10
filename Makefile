CC = gcc
CFLAGS = -Wall -Wextra -g
SRCDIR = src
BINDIR = bin
OBJDIR = obj

SRC = $(wildcard $(SRCDIR)/*.c)
OBJ = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRC))
TARGET = $(BINDIR)/main

.PHONY: all run clean

all: $(TARGET) | $(BINDIR)

run: $(TARGET) | $(BINDIR)
	$(TARGET)

$(TARGET): $(OBJ) | $(BINDIR)
	$(CC) $(CFLAGS) $^ -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

clean:
	rm -rf $(OBJ) $(OBJDIR) $(BINDIR) $(TARGET)
