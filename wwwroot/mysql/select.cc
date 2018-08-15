#include <stdio.h>
#include <mysql/mysql.h>
#include "cgi_base.h"
int main()
{
	//获取query_string
	char buf[1024*4]={0};
	if (GetQueryString(buf)<0)
	{
		fprintf(stderr, "GetQueryString faile\n");
		return 1;
	}
	//数据库查询
	//导出
	//mysql api 思路
	//1.连接到数据库
	//2.拼接sql语句
	//3.把sql语句发送到服务器
	//4。读取并遍历服务器结果
	//5.断开连接


	//1.连接到数据库
	MYSQL* connect_fd=mysql_init(NULL);
	MYSQL* connect_ret=mysql_real_connect(connect_fd,"127.0.0.1","","studentDB",3306,NULL,0);
	if (connect_ret==NULL)
	{
		fprintf(stderr, "mysql connect failed\n");
		return 1;
	}
	fprintf(stderr, "mysql connect ok!\n");
	//2.拼装sql语句
	const char* sql="select *from xinxi";
	//3.发到服务器
	int ret=mysql_query(connect_fd,sql);
	if (ret<0)
	{
		fprintf(stderr, "mysql_query failed!%s\n",sql);
		return 1;
	}
	//4.读取遍历服务器返回的结果
	MYSQL_RES* result=mysql_store_result(connect_fd);
	if (result==NULL)
	{
		fprintf(stderr, "mysql_store_result failed\n");
		return 1;
	}
	//a获取到表里有几行几列
	int rows=mysql_num_rows(result);
	int fields=mysql_num_fields(result);
	//b.获取到结果集合的表结构
	MYSQL_FIELD* field=mysql_fetch_field(result);
	
	while(field!=NULL)
	{
		printf("%s\t",field->name);
		field=mysql_fetch_field(result);
	}
	printf("<br>\n");
	//c.获取每个元素的具体值
	int i=0;
	for (int i = 0; i < ro; ++i)
	{
		MYSQL_ROW row=mysql_fetch_row(result);
		int j=0;
		for (; j < fields; ++j)
		{
			printf("%s\t", row[j]);
		}
		printf("<br>\n");
	}
	printf("<br>\n");
//5.断开连接
	mysql_close(connect_fd);

	return 0;
}