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
//  描述: 使用阿里云API，实现车牌识别
//
////////////////////////////////////////////////


#include <time.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <string.h>
#include <strings.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "cJSON.h"

#define NAMESIZE 50

void remote_info(struct hostent *he)
{
	assert(he);

	printf("主机的官方名称：%s\n", he->h_name);

	int i;
	for(i=0; he->h_aliases[i]!=NULL; i++)
	{
		printf("别名[%d]：%s\n", i+1, he->h_aliases[i]);
	}

	printf("IP地址长度：%d\n", he->h_length);

	for(i=0; he->h_addr_list[i]!=NULL; i++)
	{
		printf("IP地址[%d]：%s\n", i+1,
			inet_ntoa(*((struct in_addr **)he->h_addr_list)[i]));
	}
}


void http_request(char*buf, int size, char *base64)
{
	assert(buf);
	assert(base64);
	bzero(buf, size);

	snprintf(buf, size, "POST /api/recogliu.do HTTP/1.1\r\n"
			    "Host: anpr.sinosecu.com.cn\r\n"
			    "Authorization: APPCODE d487d937315848af80710a06f4592fee\r\n\r\n"
			    "img: %s",
			    base64);
}

void show_weather_info(char *json) // json: "{.....}"
{
	printf("json: %s\n", json);
}

void get_plate(char *base64)
{
	printf("请输入车牌图片的名称:");
	char name[30], cmd[50];
	bzero(name, 30);
	bzero(cmd,  50);

	//fgets(name, 30, stdin);
	//snprintf(cmd, 50, "./base64 < %s > plate.base64", strtok(name, "\n"));
	snprintf(cmd, 50, "./base64 < %s > plate.base64", "plate.png");
	system(cmd);

	// 将a.base64中的内容读取到base64中
	bzero(base64, 200*1024);
	int fd = open("plate.base64", O_RDONLY);
	int total = 0;
	while(1)
	{
		int n = read(fd, base64+total, 200*1024);
		if(n <= 0)
			break;

		total += n;
	}
}

int get_size(char *httphead)
{
	assert(httphead);

	char *delim = "Content-Length: ";

	char *p = strstr(httphead, delim);
	if(p != NULL)
	{
		return atoi(p+strlen(delim));
	}

	return 0;
}

int main(int argc, char **argv)
{
	char *base64 = calloc(200, 1024); // 存放图片的base64编码
	get_plate(base64);

	char *host = "anpr.sinosecu.com.cn";
	struct hostent *he = gethostbyname(host);
	if(he == NULL)
	{
		perror("gethostbyname() failed");
		exit(0);
	}

#ifdef DEBUG
	remote_info(he);
#endif

	struct in_addr **addr_list = (struct in_addr **)(he->h_addr_list);

	int fd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in srvaddr;
	socklen_t len = sizeof(srvaddr);
	bzero(&srvaddr, len);

	srvaddr.sin_family = AF_INET;
	srvaddr.sin_port   = htons(80);
	srvaddr.sin_addr   = *(struct in_addr *)(he->h_addr_list[0]);

	if(connect(fd, (struct sockaddr *)&srvaddr, len) == -1)
	{
		perror("connect() failed");
		exit(0);
	}
	printf("连接成功！\n");

	char *sndbuf = calloc(1, 200*1024);
	http_request(sndbuf, 200*1024, base64);   

#ifdef DEBUG
	printf("Request:\n");
	printf("+++++++++++++++++++++++++++++++\n");
	printf("%.1000s", sndbuf);
	printf("+++++++++++++++++++++++++++++++\n\n");
#endif

	int total_snd = strlen(sndbuf);
	int m=0;
	while(total_snd > 0)
	{
		int n = send(fd, sndbuf+m, 1024, 0);
		if(n == -1)
		{
			perror("send() failed");
			exit(0);
		}

		total_snd -= n;
		m += n;
	printf("已发送请求报文%d个字节\n", m);
	}
	free(sndbuf);

	m = 0;
	// 接收服务器响应报文（头部）
	char *head = calloc(1, 1024);
	for(int offset = 0; m > 0; offset++)
	{
		m = read(fd, head+offset, 1);
		if(m == -1)
		{
			printf("读取HTTP回应失败: %s\n", strerror(errno));
			exit(0);
		}
		if(strstr(head, "\r\n\r\n"))
			break;
	}

#ifdef DEBUG
	printf("head:%s\n", head);
#endif

	// 接收服务器响应报文（JSON实体）
	int size = get_size(head); 
	char *json = calloc(1, size);
	char *tmp = json;
	while(size > 0)
	{
		int n = read(fd, tmp, size);

		if(n == 0)
			break;
		if(n == -1)
		{
			perror("recv() failed");
			exit(0);
		}

		tmp += n;
		size-= n;
	}

#ifdef DEBUG
	printf("[%d] bytes received:%s\n\n", size, json);
#endif

	free(json);

	return 0;
}
