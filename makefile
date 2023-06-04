
CC ?= gcc
NASM ?= nasm

CFLAGS += -std=gnu99 -pedantic -Wall -Wextra

SOURCES = cpu8086.c
HELLO_COM_SOURCES = hello.asm
HELLO2_COM_SOURCES = hello2.asm

all: clean cpu8086 hello_com hello2_com

clean:
	$(RM) cpu8086

hello_com: $(HELLO_COM_SOURCES)
	$(NASM) $(HELLO_COM_SOURCES) -fbin -o hello.com

hello2_com: $(HELLO2_COM_SOURCES)
	$(NASM) $(HELLO2_COM_SOURCES) -fbin -o hello2.com

cpu8086: $(SOURCES)
	$(CC) -o cpu8086 $(SOURCES) $(CFLAGS)
