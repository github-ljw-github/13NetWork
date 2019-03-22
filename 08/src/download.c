//////////////////////////////////////////////////////////////////
//
//  Copyright(C), 2013-2016, GEC Tech. Co., Ltd.
//
//  File name: teddy/download.c
//
//  Author: Vincent Lin (林世霖)  微信公众号：秘籍酷
//
//  Date: 2017-8
//  
//  Description: HTTP下载器（支持断点续传）
//
//  GitHub: github.com/vincent040   Bug Report: 2437231462@qq.com
//
//////////////////////////////////////////////////////////////////

#include "common.h"

int g_connecting = CONNECTING;
int g_searching  = SEARCHING;

sem_t s;
char *filename;

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

void progress(long long nread, long long filesize)
{
	struct winsize ws;
	ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);

	int bar_len = ws.ws_col-32;
	bar_len = bar_len > 60 ? 60 : bar_len;

	int rate = filesize/bar_len;
	int cur  = nread/rate;

	char empty[bar_len]; 
	empty[0] = '\r';
	for(int i=1; i<bar_len; i++)
	{
		empty[i] = ' ';	
	}
	fprintf(stderr, "%s", empty);

	char *total = calloc(1, 16);
	if(filesize < 1024)
		snprintf(total, 16, "%llu", filesize);
	else if(filesize >= 1024 && filesize < 1024*1024)
		snprintf(total, 16, "%.1fKB", (float)filesize/1024);
	else if(filesize >= 1024*1024 && filesize < 1024*1024*1024)
		snprintf(total, 16, "%.1fMB", (float)filesize/(1024*1024));


	char *bar = calloc(1, 128);
	if(nread < 1024)
		snprintf(bar, 128, "\r[%llu/%s] [", nread, total);
	else if(nread < 1024*1024)
		snprintf(bar, 128, "\r[%.1fKB/%s] [", (float)nread/1024, total);
	else if(nread < 1024*1024*1024)
		snprintf(bar, 128, "\r[%.1fMB/%s] [", (float)nread/(1024*1024), total);
	free(total);

	int i;
	for(i=0; i<cur; i++)
		snprintf(bar+strlen(bar), 128-strlen(bar)-i, "%s", "#");

	for(i=0; i<bar_len-cur-1; i++)
		snprintf(bar+strlen(bar), 128-strlen(bar)-i, "%s", "-");

	snprintf(bar+strlen(bar), 128-strlen(bar),
			"] [%.1f%%]%c", (float)nread/filesize*100,
			nread==filesize?'\n':' ');
	fprintf(stderr, "%s", bar);
	free(bar);
}

void arg_parser(char *arg, char **host, char **file)
{
	assert(arg);
	assert(host);
	assert(file);

	if(arg[strlen(arg)-1] == '/')
	{
		fprintf(stderr, "非法链接.\n");
		exit(0);
	}

	char *h, *f;
	h = f = arg;

	char *delim1 = "http://";
	char *delim2 = "https://";
	if(strstr(arg, delim1) != NULL)
	{
		h += strlen(delim1);
	}
	else if(strstr(arg, delim2) != NULL)
	{
		h += strlen(delim2);
	}

	f = strstr(h, "/");
	if(f == NULL)
	{
		fprintf(stderr, "非法链接.\n");
		exit(0);
	}
	f += 1;

	*host = calloc(1, 256);
	*file = calloc(1, 2048);

	memcpy(*host, h, f-h-1);
	memcpy(*file, f, strlen(f));
}

void *searching_host(void *arg)
{
	pthread_detach(pthread_self());

	fprintf(stderr, "正在寻找主机");
	while(g_searching == SEARCHING)
	{
		fprintf(stderr, ".");
		usleep(100*1000);
	}
	printf("%s", g_searching==SEARCH_SUCCESS ? "[OK]\n": "[FAIL]\n");
	sem_post(&s);

	pthread_exit(NULL);
}

void *connecting_host(void *arg)
{
	pthread_detach(pthread_self());

	fprintf(stderr, "正在连接远端服务器");
	while(g_connecting == CONNECTING)
	{
		fprintf(stderr, ".");
		usleep(100*1000);
	}
	printf("%s", g_connecting==CONNECT_SUCCESS ? "[OK]\n": "[FAIL]\n");
	sem_post(&s);

	pthread_exit(NULL);
}

int main(void)
{
	printf("请输入下载链接[直接按回车下载boa]：\n");
	char *link = calloc(1, 1024);
	fgets(link, 1024, stdin);

	if(link[0] == '\n')
	{
		strcpy(link,
			"http://www.boa.org/boa-0.94.14rc21.tar.gz");
	}
	else
	{
		link = strtok(link, "\n");
	}


	char *host = NULL;
	char *filepath = NULL;
	arg_parser(link, &host, &filepath);
	free(link);

	// 显示连接服务器的等待过程... ...
	sem_init(&s, 0, 0);
	pthread_t tid;
	pthread_create(&tid, NULL, searching_host, NULL);
	struct hostent *he = gethostbyname(host);

	if(he == NULL && errno == HOST_NOT_FOUND)
	{
		g_searching = SEARCH_FAIL;
		sem_wait(&s);
		fprintf(stderr, "主机名有误，或DNS没配置好.\n");
		exit(0);
	}
	else if(he == NULL)
	{
		g_searching = SEARCH_FAIL;
		sem_wait(&s);
		fprintf(stderr, "非法链接.\n");
		exit(0);
	}
	g_searching = SEARCH_SUCCESS;
	sem_wait(&s);

#ifdef DEBUG
	printf("host: %s\n", host);
	printf("filepath: %s\n", filepath);

	remote_info(he);
#endif

	// 取得站点主机IP并发起连接请求
	struct in_addr **addr_list = (struct in_addr **)(he->h_addr_list);

	int fd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in srvaddr;
	socklen_t addrlen = sizeof(srvaddr);
	bzero(&srvaddr, addrlen);

	srvaddr.sin_family = AF_INET;
	srvaddr.sin_port   = htons(80);
	srvaddr.sin_addr   = *addr_list[0];

	long state = fcntl(fd, F_GETFL);
	state |= O_NONBLOCK;
	fcntl(fd, F_SETFL, state);

	g_connecting = CONNECTING;
	pthread_create(&tid, NULL, connecting_host, NULL);
	if(connect(fd, (struct sockaddr *)&srvaddr, addrlen) == -1
		&& EINPROGRESS)
	{
		fd_set wset;
		FD_ZERO(&wset);
		FD_SET(fd, &wset);

		struct timeval tv = {10, 0};

		long state = fcntl(fd, F_GETFL);
		state &= ~O_NONBLOCK;
		fcntl(fd, F_SETFL, state);

		int n = select(fd+1, NULL, &wset, NULL, &tv);
		if(n < 0)
		{
			perror("select() failed");
			exit(0);
		}
		else if(n == 0)
		{
			g_connecting = CONNECT_FAIL;
			sem_wait(&s);
			fprintf(stderr, "连接超时.\n");
			exit(0);
		}
		else if(FD_ISSET(fd, &wset))
		{
			if(connect(fd, (struct sockaddr *)&srvaddr, addrlen) == 0)
			{
				g_connecting = CONNECT_SUCCESS;
			}
			else
			{
				perror("connect error");
				exit(0);
			}
		}
	}

	// 准备好本地文件，如果已下载了部分内容，则计算还需下载的字节量
	FILE *fp = NULL;
	long long curlen = 0LL;

	if(strstr(filepath, "/"))
		filename = strrchr(filepath, '/')+1;
	else
		filename = filepath;

	if(access(filename, F_OK))
	{
		fp = fopen(filename, "w");
	}
	else
	{
		struct stat fileinfo;
		stat(filename, &fileinfo);

		curlen = fileinfo.st_size;
		fp = fopen(filename, "a");
	}
	if(fp == NULL)
	{
		perror("fopen() failed");
		exit(0);
	}
	setvbuf(fp, NULL, _IONBF, 0);

	// 给站点发送HTTP请求报文
	char *sndbuf = calloc(1, 1024);
	http_request(sndbuf, 1024, filepath, host, curlen);   

	int n = send(fd, sndbuf, strlen(sndbuf), 0);
	if(n == -1)
	{
		perror("send() failed");
		exit(0);
	}

	sem_wait(&s);
#ifdef DEBUG
	printf("Request:\n");
	printf("+++++++++++++++++++++++++++++++\n");
	printf("%s", sndbuf);
	printf("+++++++++++++++++++++++++++++++\n");
	printf("[%d] bytes have been sent.\n\n", n);
#endif
	free(sndbuf);

	long long total_bytes = curlen;

	// 读取站点返回的HTTP响应报文
	char *httphead = calloc(1, 2048);
	n = 0;
	while(1)
	{
		read(fd, httphead+n, 1);
		n++;
		if(strstr(httphead, "\r\n\r\n"))
			break;
	}

#ifdef DEBUG
	printf("Response HTTP header:\n");
	printf("*******************************\n");
	printf("%s", httphead);
	printf("\n*******************************\n");
#endif

	long long size; // 要下载的文件的总大小
	long long len ; // 本次要下载的大小
	size = get_size(httphead);
	len  = get_len(httphead);

	// 检查HTTP响应报文
	// 返回：真：服务器正常返回
	//       假：发生错误
	if(!check_response(httphead))
	{
		// 文件已经下载
		if(curlen == size && curlen != 0)
		{
			fprintf(stderr, "该文件已下载.\n");
		}

		// 其他错误
		if(!access(filename, F_OK) && curlen == 0)
			remove(filename);

		exit(0);
	}

	free(httphead);

	// 读取下载的文件内容
	char *recvbuf  = calloc(1, 1024);
	while(1)
	{
		n = recv(fd, recvbuf, 1024, 0);

		if(n == 0)
			break;

		if(n == -1)
		{
			perror("recv() failed");
			exit(0);
		}

		fwrite(recvbuf, n, 1, fp);

		total_bytes += n;
		progress(total_bytes, size);

		if(total_bytes >= size)
		{
			printf("[Done]\n");
			break;
		}
	}

	fclose(fp);
	free(recvbuf);

	exit(0);
}
