CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -g
OBJDIR = obj
SRCDIR = src
BINDIR = bin

OBJS = $(OBJDIR)/main.o \
       $(OBJDIR)/input.o \
       $(OBJDIR)/prompt.o \
       $(OBJDIR)/parser.o \
       $(OBJDIR)/hop.o \
       $(OBJDIR)/reveal.o \
       $(OBJDIR)/log.o \
       $(OBJDIR)/execute.o \
       $(OBJDIR)/background.o \
       $(OBJDIR)/ping.o \


TARGET = shell.out

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR) $(TARGET)

.PHONY: all clean
