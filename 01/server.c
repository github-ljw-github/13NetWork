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

int main(int argc, char **argv)
{
	int sockfd = Socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	bzero(&addr, len);

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(50001);

	Bind(sockfd, (struct sockaddr *)&addr, len);
	Listen(sockfd, 3);

	printf("等待连接...\n");
	int fd = Accept(sockfd, NULL, NULL);
	printf("连接成功！\n");

	char buf[100];
	while(1)
	{
		bzero(buf, 100);
		if(read(fd, buf, 100) == 0)
			break;

		printf("收到: %s", buf);
		write(fd, buf, strlen(buf));
	}

	close(fd);
	close(sockfd);

	return 0;
}
