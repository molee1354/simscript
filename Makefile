CC = gcc
CFLAGS = -Wall -Wextra
DEBUG_CFLAGS = -g
RELEASE_CFLAGS = -O3
SRCDIR = src
BINDIR = bin
OBJDIR = obj
SRC = $(wildcard $(SRCDIR)/*.c)
OBJ = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRC))
DEBUG_TARGET = $(BINDIR)/debug
RELEASE_TARGET = $(BINDIR)/simscript

.PHONY: all debug release install clean

all: release

debug: $(DEBUG_TARGET) | $(BINDIR)

release: $(RELEASE_TARGET) | $(BINDIR)
	@ cp $(RELEASE_TARGET) ./

install: release
	@ sudo cp $(RELEASE_TARGET) /usr/local/bin

$(DEBUG_TARGET): $(OBJ) | $(BINDIR)
	$(CC) $(CFLAGS) $(DEBUG_CFLAGS) $^ -o $@

$(RELEASE_TARGET): $(OBJ) | $(BINDIR)
	$(CC) $(CFLAGS) $(RELEASE_CFLAGS) $^ -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) $(DEBUG_CFLAGS) -c $< -o $@

$(OBJDIR):
	@ mkdir -p $(OBJDIR)

$(BINDIR):
	@ mkdir -p $(BINDIR)

clean:
	@ rm -rf $(OBJ) $(OBJDIR) $(BINDIR); \
	if [ -e simscript ]; then \
		rm simscript; \
	fi
