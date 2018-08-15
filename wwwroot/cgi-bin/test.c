//此处实现的可执行程序就是cgi程序
#include <stdio.h>
#include <stdlib.h>
#include "cgi_base.h"
int main()
{
	char buf[1024*4]={0};
	int ret=GetQueryString(buf);
	if (ret<0)
	{
		fprintf(stderr, "GetQueryString failed\n");
		return 1;
	}
	int a,b;
	sscanf(buf,"a=%d&b=%d",&a,&b);
	int sum=a+b;
	//printf输出的结果就返回到客户端上
	//作为HTTP服务器，每次给客户端返回的字符串必须符合HTTP协议的格式
	//由于父进程已经把首行，header空行都已经写回给客户端
	//因此此时CGI程序只返回body
	printf("<h1>sum=%d</h1>",sum);
	return 0;
}
