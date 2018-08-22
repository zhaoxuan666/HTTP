#pragma once
#define SIZE 10240
#include <stdio.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <sys/stat.h>
typedef struct Request
{
	 char first_line[SIZE];
	 char *method;
	 char *url;
	 char *url_path;//重点1
	 char *query_string;//重点2
	 //char *version;

	 //header 部分，完整的解析需要使用二叉树搜索数，hash表，为了简单只保留Conten-length
	 int content_length;
	 





}Request;
