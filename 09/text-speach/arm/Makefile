CROSS = arm-none-linux-gnueabi-
CC = $(CROSS)gcc

client.elf:
	$(CC) client.c  wrap.c  -o $@

clean:
	-@$(RM) client.elf
	-@echo "Done."
