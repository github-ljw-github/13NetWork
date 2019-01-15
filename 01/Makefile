SRC := $(wildcard *.c)
ELF := $(patsubst %.c, %.elf, $(SRC))
ELF := $(filter-out wrap.elf, $(ELF))

CPPFLAGS += -I inc/
LDFLAGS  += -L lib/

#CROSS = arm-none-linux-gnueabi-
CC = $(CROSS)gcc

all:$(ELF)

$(ELF):%.elf:%.c wrap.c
	$(CC) -o $@ $^ $(CPPFLAGS) $(LDFLAGS)

clean:
	-@$(RM) $(ELF) .*.sw?
	-@echo "Done."

.PHONY:all clean
