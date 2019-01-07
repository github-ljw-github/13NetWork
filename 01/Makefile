SRC = $(wildcard *.c)
ELF = $(patsubst %.c, %.elf, $(SRC))

CPPFLAGS += -I inc/

LDFLAGS  += -L lib/
LDFLAGS  += -lwrap
LDFLAGS  += -Wl,-rpath=./lib

#CROSS = arm-none-linux-gnueabi-
CC = $(CROSS)gcc

all:$(ELF)

$(ELF):%.elf:%.c
	$(CC) -o $@ $^ $(CPPFLAGS) $(LDFLAGS)

clean:
	-@$(RM) $(ELF) .*.sw?
	-@echo "Done."

.PHONY:all clean
