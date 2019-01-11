vpath %.c = src/
vpath %.h = inc/

SRC += $(wildcard *.c)

#CROSS = arm-none-linux-gnueabi-
CC = $(CROSS)gcc

CPPFLAGS += -I cJSON/

LDFLAGS += -L cJSON/
LDFLAGS += -lcjson
LDFLAGS += -Wl,-rpath=./cJSON


weather.elf:$(SRC)
	$(CC) $^ -o $@ $(CPPFLAGS) $(LDFLAGS) -DNDEBUG

debug.elf:$(SRC)
	$(CC) $^ -o $@ $(CPPFLAGS) $(LDFLAGS) -DDEBUG

clean:
	$(RM) *.elf

distclean:clean
	$(RM) core .*.sw? *.html
