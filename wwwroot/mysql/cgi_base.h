#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


//分GET/POST俩种情况读取计算的参数
//1.GET从query_string 读
//2.POST从body中读
//读取的结果放到buf里
static int GetQueryString(char buf[])
{
		//1.从环境变量中获取到的方法
	char* method=getenv=("REQUEST_METHOD");
	if (method==NULL)
	{
		//由于当前的CGI程序对应的标准输出已经被重定向到管道上了，
		//而这部分数据又会被返回给客户端，
		//避免让程序内部的错误暴露给普通用户，通过stderr来作为输入日志的手段
		//避免让程序内部的错误暴露给普通用户
		fprintf(stderr,"error\n");
		return -1;
	}
	//2.判定方法为GET还是POST
	//如果是GET就从环境变量里读取 QUERY_STRING
	//如果是POST，就需要从环境变量里读取CONTENT_LENGTH
	char buf[1024*10]={}
	if (strcasecmp(method,"GET")==0)
	{
		char *query_string=getenv("CONTENT_LENGTH");
		if(query_string==NULL){
			fprintf(stderr, "query_string is NULL\n");
			return -1;
		}//拷贝完后，buf里的内容形如  a=10&b=20
		strcpy(buf,query_string);
	}
	else{
		char *content_lenght_str=getenv("CONTENT_LENGTH")
		if(content_lenght_str==NULL)
		{
			fprintf(stderr, "content_lenght is NULL\n");
			return -1;
		}
		int content_lenght=atoi(content_lenght_str);
		int i=0;
		for (int i = 0; i < content_lenght; ++i)
		{//此处 由于父进程把body写入管道
			//子进程又已经把0号文件描述符重定向到了管道
			//此时从标准输入来读数据，也就读到了管道中的数据
			read(0,&buf[i],1);
		}
		//此循环读完后，buf里内容就形如
		//a=10&b=30
		buf[i]='\0';

	}
	return 0;
}