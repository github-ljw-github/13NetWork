vpath %.c = src/
vpath %.h = include/

CPPFLAGS += -I./include
LDFLAGS  += -L./libs/x64/ -lmsc

tts: network.c tts.c wrap.c
	g++ $^ -o bin/tts $(CPPFLAGS) $(LDFLAGS)

clean:
	-@$(RM) bin/tts
