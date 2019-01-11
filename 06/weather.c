///////////////////////////////////////////////////////////
//
//  Copyright(C), 2005, GEC Tech. Co., Ltd.
//
//  文件: forecast/weather.c
//  描述: 从阿里云API市场获取实时天气预报信息
//
//  作者: Vincent Lin (林世霖)  微信公众号：秘籍酷
//  技术微店: http://weidian.com/?userid=260920190
//  技术交流: 260492823（QQ群）
//
///////////////////////////////////////////////////////////


#include <time.h>
#include <stdio.h>
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

#define CODESIZE 8

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


void http_request(char*buf, int size, char *city)
{
	assert(buf);
	assert(city);

	bzero(buf, size);

	snprintf(buf, size, "GET /spot-to-weather?"
			    "area=%s "
			    "HTTP/1.1\r\n"
			    "Host: saweather.market.alicloudapi.com\r\n"
			    "Authorization: "
			    "APPCODE d487d937315848af80710a06f4592fee\r\n\r\n",
			    city);
}

void show_weather_info(char *json) // json: "{.....}"
{
	printf("json: %s\n", json);
	/*
	cJSON *root    = cJSON_Parse(json);
	cJSON *result  = cJSON_GetObjectItem(root, "result");

	if(cJSON_GetObjectItem(body, "ret_code")->valueint == -1)
	{
		printf("%s\n",cJSON_GetObjectItem(body, "remark")->valuestring);	
		return;
	}

	cJSON *now      = cJSON_GetObjectItem(body, "now");
	cJSON *cityInfo = cJSON_GetObjectItem(body, "cityInfo");
	cJSON *today    = cJSON_GetObjectItem(body, "f1");
	cJSON *tomorrow = cJSON_GetObjectItem(body, "f2");
	cJSON *day_3rd  = cJSON_GetObjectItem(body, "f3");

	char *country = cJSON_GetObjectItem(cityInfo, "c9")->valuestring;
	char *province= cJSON_GetObjectItem(cityInfo, "c7")->valuestring;
	char *city    = cJSON_GetObjectItem(cityInfo, "c5")->valuestring;

	bool zhixiashi = !strcmp(city, province);
	printf("城市：%s·%s%s%s\n\n", country, province,
			zhixiashi ? "" : "·", zhixiashi ? "" : city);

	printf("现在天气：%s\n",    cJSON_GetObjectItem(now, "weather")->valuestring);
	printf("现在气温：%s°C\n\n",cJSON_GetObjectItem(now, "temperature")->valuestring);

	printf("明天天气：%s\n",   cJSON_GetObjectItem(tomorrow, "day_weather")->valuestring);
	printf("日间气温：%s°C\n", cJSON_GetObjectItem(tomorrow, "day_air_temperature")->valuestring);
	printf("夜间气温：%s°C\n\n", cJSON_GetObjectItem(tomorrow, "night_air_temperature")->valuestring);

	printf("后天天气：%s\n",     cJSON_GetObjectItem(day_3rd, "day_weather")->valuestring);
	printf("日间气温：%s°C\n",   cJSON_GetObjectItem(day_3rd, "day_air_temperature")->valuestring);
	printf("夜间气温：%s°C\n\n", cJSON_GetObjectItem(day_3rd, "night_air_temperature")->valuestring);
	*/
}

char *get_city(char *city)
{
	while(1)
	{
		// 遇到错误或者ctrl+d直接退出
		if(fgets(city, CODESIZE, stdin) == NULL)
			break;

		// 直接按回车则进入循环
		if(city[0] != '\n')
			break;
	}

	int i;
	for(i=0; i<CODESIZE; i++)
	{
		if(!isspace(city[i]))
			break;
	}
	char *begin = city+i;

	for(;i<CODESIZE; i++)
	{
		if(isspace(city[i]))	
			break;
	}
	char *end = city+i-1;

	memcpy(city, begin, end-begin+1);

	int len = end - begin + 1;
	bzero(city+len, CODESIZE-len);
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
	printf("请输入要查询的城市: ");

	char city[CODESIZE];
	get_city(city);

	char *host = "saweather.market.alicloudapi.com";
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
	//srvaddr.sin_addr   = *addr_list[0];
	srvaddr.sin_addr   = *(struct in_addr *)(he->h_addr_list[0]);

	if(connect(fd, (struct sockaddr *)&srvaddr, len) == -1)
	{
		perror("connect() failed");
		exit(0);
	}

	char *sndbuf = calloc(1, 1000);
	http_request(sndbuf, 1000, city);   

#ifdef DEBUG
	printf("Request:\n");
	printf("+++++++++++++++++++++++++++++++\n");
	printf("%s", sndbuf);
	printf("+++++++++++++++++++++++++++++++\n\n");
#endif

	int n = send(fd, sndbuf, strlen(sndbuf), 0);
	if(n == -1)
	{
		perror("send() failed");
		exit(0);
	}
	free(sndbuf);

	int m = 0;
	// 接收服务器响应报文（头部）
	char *head = calloc(1, 1024);
	for(int offset = 0; m > 0; offset++)
	{
		m = read(fd, head+offset, 1);
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
		n = read(fd, tmp, size);

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

	show_weather_info(json);
	free(json);

	return 0;
}
