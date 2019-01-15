////////////////////////////////////////////////
//
//  Copyright(C), 广州粤嵌通信科技股份有限公司
//
//  作者: Vincent Lin (林世霖)
//
//  微信公众号: 秘籍酷
//  技术交流群: 260492823（QQ群）
//  GitHub链接: https://github.com/vincent040
//
//  描述: 简单的TCP通信示例代码
//
////////////////////////////////////////////////

#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include "wrap.h"

void usage(int argc, char **argv)
{
	if(argc != 3)
	{
		printf("用法: %s <服务器IP> <服务器端口>\n", argv[0]);
		exit(0);
	}
}

int main(int argc, char **argv)
{
	usage(argc, argv);

	int sockfd = Socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	bzero(&addr, len);

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(argv[1]);
	addr.sin_port = htons(atoi(argv[2]));

	Connect(sockfd, (struct sockaddr *)&addr, len);
	printf("连接成功！\n");

	char sndbuf[100];
	char rcvbuf[100];
	while(1)
	{
		bzero(sndbuf, 100);
		fgets(sndbuf, 100, stdin);
		write(sockfd, sndbuf, strlen(sndbuf));

		bzero(rcvbuf, 100);
		read(sockfd, rcvbuf, 100);
		printf("来自服务器消息: %s", rcvbuf);
	}

	close(sockfd);

	return 0;
}
