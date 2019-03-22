#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>
#include <strings.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>

#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/fb.h>
#include <linux/un.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include "common.h"

int init_sock()
{
	int fd = Socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	bzero(&addr, len);
	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port        = htons(50005);

	Bind(fd, (struct sockaddr *)&addr, len);
	Listen(fd, 5);

	return fd;
}

char *get_text(int connfd)
{
	static char text[1024];

	bzero(text, 1024);
	if(read(connfd, text, 1024) <= 0)
		return NULL;

	return text;
}

void send_wav(int connfd, const char *wav, int size)
{
	int fd = open(wav, O_RDONLY);

	// 1，发送wav尺寸信息
	write(connfd, &size, 4);

	// 2，发送wav具体内容
	char *data = (char *)calloc(1, 1024);
	int total = 0;
	while(1)
	{
		bzero(data, 1024);
		int n = read(fd, data, 1024);
		if(n <= 0)
			break;

		int m = write(connfd, data, n);
		total += m;
	}
	printf("语音【%d】字节\n", total);
}

