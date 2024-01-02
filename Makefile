CC = clang
WINCC = x86_64-w64-mingw32-gcc
CFLAGS = -Wall -Wextra -Wno-unused-command-line-argument -lm
DEBUG_CFLAGS := -g
RELEASE_CFLAGS := -O3

SRCDIR = src
SRC_LIBDIR := $(SRCDIR)/libs
SRC_OBJDIR := $(SRCDIR)/objs

OBJDIR = obj
OBJ_LIBDIR := $(OBJDIR)/libs
OBJ_OBJDIR := $(OBJDIR)/objs

BINDIR = bin
TESTDIR := scripts
INSTDIR := /usr/local/bin

SRC = $(wildcard $(SRCDIR)/*.c)
SRC_LIB := $(wildcard $(SRC_LIBDIR)/*.c)
SRC_OBJ := $(wildcard $(SRC_OBJDIR)/*.c)

OBJ := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRC)) \
   	  $(patsubst $(SRC_LIBDIR)/%.c,$(OBJ_LIBDIR)/%.o,$(SRC_LIB)) \
   	  $(patsubst $(SRC_OBJDIR)/%.c,$(OBJ_OBJDIR)/%.o,$(SRC_OBJ))

TARG = simscript
DEBUG_TARGET := $(BINDIR)/debug
RELEASE_TARGET := $(BINDIR)/$(TARG)
RELEASE_TARGET_WIN := $(BINDIR)/$(TARG).exe

.PHONY: all debug test release install uninstall clean windows

all: release

debug: CFLAGS += $(DEBUG_CFLAGS)

debug: $(DEBUG_TARGET) | $(BINDIR)

test: release
	@ $(TESTDIR)/test.sh

release: $(RELEASE_TARGET) | $(BINDIR)
	@ cp $(RELEASE_TARGET) ./

install: release
	@ printf "Copying simscript binary to [%s]\n" $(INSTDIR); \
	sudo cp $(RELEASE_TARGET) $(INSTDIR) && \
	printf "\033[1;32mINSTALL SUCCESS %s -> [%s/%s]\033[0m\n\n" $(RELEASE_TARGET) $(INSTDIR) $(TARG)

uninstall: clean
	@ printf "\033[1;33mRemoving\033[0m simscript binary from [%s]\n" $(INSTDIR); \
	sudo rm $(INSTDIR)/$(TARG) && \
	printf "\033[1;32mUNINSTALL SUCCESS\033[0m\n\n"

$(DEBUG_TARGET): $(OBJ) | $(BINDIR)
	$(CC) $(CFLAGS) $(DEBUG_CFLAGS) $^ -o $@

$(RELEASE_TARGET): $(OBJ) | $(BINDIR)
	@ printf "\033[1;32mBUILD SUCCESS\t[%s]\033[0m\n\n" $@; \
	$(CC) $(CFLAGS) $(RELEASE_CFLAGS) $^ -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	@ printf "%-8s : %-16s -->  %s\n" "compile" $< $@; \
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(SRC_LIBDIR)/%.c | $(OBJDIR)
	@ printf "%-8s : %-16s -->  %s\n" "compile" $< $@; \
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(SRC_OBJDIR)/%.c | $(OBJDIR)
	@ printf "%-8s : %-16s -->  %s\n" "compile" $< $@; \
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	@ mkdir -p $(OBJDIR) $(OBJ_LIBDIR) $(OBJ_OBJDIR)

$(BINDIR):
	@ mkdir -p $(BINDIR)

clean:
	@ echo "Cleaning..."; \
	rm -rf $(OBJ) $(OBJDIR) $(DEBUG_TARGET); \
	if [ -e $(TARG) ]; then \
		rm $(TARG); \
	fi

windows: CC = $(WINCC)

windows: $(RELEASE_TARGET_WIN)

$(RELEASE_TARGET_WIN): $(OBJ) | $(BINDIR)
	@ printf "\033[1;32mBUILD SUCCESS\t[%s]\033[0m\n\n" $@; \
	$(CC) $(CFLAGS) $(RELEASE_CFLAGS) $^ -o $@
