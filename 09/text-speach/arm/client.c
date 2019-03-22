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

int main(int argc, char **argv) // ./client 192.168.xx.xx
{
	if(argc != 2)
	{
		printf("用法: %s <服务器IP>\n", argv[0]);
		exit(0);
	}

	// 1，创建TCP套接字
	int fd = Socket(AF_INET, SOCK_STREAM, 0);

	// 2，准备好对方的地址
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	bzero(&addr, len);

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(argv[1]);
	addr.sin_port = htons(50005);

	// 3，对对方发起连接请求..
	Connect(fd, (struct sockaddr *)&addr, len);

	// 4，不断给对方发消息
	char *buf = calloc(1, 100);
	char *wav = calloc(1, 1024);
	while(1)
	{
		// 发送待合成文本
		bzero(buf, 100);
		fgets(buf, 100, stdin);

		if(strlen(buf) <= 1)
			continue;
		Write(fd, buf, strlen(buf));


		int wavfd = open("a.wav", O_CREAT | O_RDWR | O_TRUNC, 0777);
		if(wavfd < 0)
		{
			perror("open failed");
			exit(0);
		}


		// 1，接收wav尺寸
		int size;
		Read(fd, &size, sizeof(size));
		printf("待接收音频尺寸:%d\n", size);

		// 2，接收音频数据
		int total = 0;
		while(size > 0)
		{
			bzero(wav, 1024);
			int n = Read(fd, wav, size);

			if(n <= 0)
				break;

			total += n;
			size  -= n;

			write(wavfd, wav, n);
		}
		close(wavfd);
		printf("已接收音频尺寸:%d\n", total);

		system("aplay a.wav");
	}

	return 0;
}
