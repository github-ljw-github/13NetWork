vpath %.c = src/
vpath %.h = inc/

SRC += $(wildcard src/*.c)

#CROSS = arm-none-linux-gnueabi-
CC = $(CROSS)gcc

CPPFLAGS += -I inc/

LDFLAGS += -lpthread


download.elf:$(SRC)
	$(CC) $^ -o $@ $(CPPFLAGS) $(LDFLAGS) -DNDEBUG

debug.elf:$(SRC)
	$(CC) $^ -o $@ $(CPPFLAGS) $(LDFLAGS) -DDEBUG

clean:
	$(RM) *.elf

distclean:clean
	$(RM) core .*.sw? *.html src/*.o src/.*.sw?
