CC = gcc
WINCC = x86_64-w64-mingw32-gcc
CFLAGS = -Wall -Wextra
DEBUG_CFLAGS = -g
RELEASE_CFLAGS = -O3
SRCDIR = src
BINDIR = bin
OBJDIR = obj
FILEDIR = files
TESTDIR = tests
SRC = $(wildcard $(SRCDIR)/*.c)
OBJ = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRC))
DEBUG_TARGET = $(BINDIR)/debug
RELEASE_TARGET = $(BINDIR)/simscript
RELEASE_TARGET_WIN = $(FILEDIR)/simscript.exe

.PHONY: all debug test release install clean windows

all: release

debug: $(DEBUG_TARGET) | $(BINDIR)

test: release
	@ $(TESTDIR)/test.sh

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

$(FILEDIR):
	@ mkdir -p $(FILEDIR)

clean:
	@ rm -rf $(OBJ) $(OBJDIR) $(BINDIR); \
	if [ -e simscript ]; then \
		rm simscript; \
	fi

windows: CC = $(WINCC)
windows: $(RELEASE_TARGET_WIN)

$(RELEASE_TARGET_WIN): $(OBJ) | $(FILEDIR)
	$(CC) $(CFLAGS) $(RELEASE_CFLAGS) $^ -o $@
