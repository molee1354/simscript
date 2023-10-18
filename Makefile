CC = gcc
CFLAGS = -Wall -Wextra
DEBUG_CFLAGS = -g
RELEASE_CFLAGS = -O3

SRCDIR = src
BINDIR = bin
OBJDIR = obj

SRC = $(wildcard $(SRCDIR)/*.c)
OBJ = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRC))

BINARY = simscript
DEBUG_TARGET = $(BINDIR)/debug
RELEASE_TARGET = $(BINDIR)/$(BINARY)

.PHONY: all debug release install clean uninstall

all: release

debug: $(DEBUG_TARGET) | $(BINDIR)

release: $(RELEASE_TARGET) | $(BINDIR)
	@ cp $(RELEASE_TARGET) ./

install: release
	@ echo -e "\nInstalling simscript..."; \
	echo -e "'$(BINARY)' -> '/usr/local/bin/$(BINARY)'..."; \
	sudo cp $(RELEASE_TARGET) /usr/local/bin/$(BINARY)

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
	@ echo "Cleaning directory..."; \
	rm -rf $(OBJ) $(OBJDIR) $(BINDIR); \
	if [ -e simscript ]; then \
		rm simscript; \
	fi

uninstall: clean
	@ echo "Removing simscript from system..."; \
	sudo rm -f /usr/local/bin/$(BINARY)
