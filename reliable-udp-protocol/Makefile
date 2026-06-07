CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LDFLAGS = -lcrypto


UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	CFLAGS += -I$(shell brew --prefix openssl)/include
	LDFLAGS += -L$(shell brew --prefix openssl)/lib
endif

all: server client

server: server.c sham_utils.c sham.h
	$(CC) $(CFLAGS) -o server server.c sham_utils.c $(LDFLAGS)

client: client.c sham_utils.c sham.h
	$(CC) $(CFLAGS) -o client client.c sham_utils.c $(LDFLAGS)

clean:
	rm -f server client *.log *.txt

.PHONY: all clean