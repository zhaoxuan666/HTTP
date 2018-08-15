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
#include "http_server.h"
typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;

int ReadLine(int sock,char buf[],ssize_t size)
{//一次从sock中读取一行数据 ，放到buf  失败返回-1
  //1,从sock一次度一个字符
  char c='\0';
  ssize_t i=0;//当前读了多少个
  while(i < size-1 && c != '\n')
  {
    ssize_t read_size=recv(sock,&c,1,0);
    if(read_size<0)
    {
      return -1;
    }
    if(read_size==0)
    {//预期要读 \n 还没读到 就读到了EOF 这样认为失败
      return -1;
    }
    if (c=='\r')
    {
      /*判断下个是否为\n*/
      recv(sock,&c,1,MSG_PEEK);//MSG_PEEK是从内核中i哦那个的缓冲区读数据，读到的数据不会从缓冲区删除
      if(c=='\n')
      {
        //此时分隔符为\r\n
        recv(sock,&c,1,0);
      }else{
        //当前是分隔符确定是\r, 转换为\n
        c='\n';
      }
    }
    //只要c读到了\r if后都变成\n
    buf[i++]=c;
  }
  buf[i]='\0';
  return i;//i为真正放缓冲区的个数
  //2.如果读到换行\n 就返回
  //\n \r \r\n
}
int Split(char input[],const char *split_char,char *output[],int output_size)//	切分函数
{
  //使用strtok函数
  char *pch;
  int i=0;
  //pch=strtok(input,split_char);
  char *tmp=NULL;//保存临时的解决线程不安全
  pch=strtok_r(input,split_char,&tmp);
  while(pch!=NULL)
  {
    if(i>=output_size)
    {
      return i;
    }
    output[i++]=pch;
    pch=strtok_r(NULL,split_char,&tmp);
  }
  return i;
}
//解析首行
int ParseFirstLine(char first_line[],char ** p_url,char **p_method)//二级指针为了修改指向
{
  //把首行按照空格进行切分
  char *tok[10];

  int tok_size=Split(first_line," ",tok,10);
  if (tok_size!=3)
  {
    /* code */
    printf("Split fail %d\n",tok_size);
    return -1;
  }
  *p_method=tok[0];
  *p_url=tok[1];
  return 0;
}
int ParseQueryString(char *url,char **p_url_path,char **p_query_string)
{
  *p_url_path=url;
  char *p=url;
  for (;*p!='\0' ; ++p)
  {
    /* code */
    if(*p=='?')
    {
      *p='\0';
      *p_query_string=p+1;
      return 0;
    }
  }
  *p_query_string=NULL;
  //循环结束都没找到? 这个亲求不带query_string
  return 0;
}
int ParsHeader(int sock,int *content_length)
{
  //1.循环从socket中读取一行
  //2.判定当前行是不是conten_length
  //读到空行结束
  char buf[SIZE]={0};
  while(1)
  {
    ssize_t read_size=ReadLine(sock,buf,sizeof(buf));
    if (read_size<=0)
    {
      return -1;
      /* code */
    }
    //处理读完的情况
    if (strcmp(buf,"\n")==0)
    {
      /* code */
      return 0;
    }
    //处理读到的数据是不是content-length
    if (content_length!=NULL && strncmp(buf,"Content-Length: ",16)==0)
    {
      *content_length=atoi(buf+16);

    }
  }
  return 0;
}
void Hander404(int sock)
{
  //构造一i个完整的HHTTP响应
  //状态吗 404
  //body部分应该也是错误页面
  const char* first_line="HTTP/1.1 404 Not Found\n";
  const char* black_line="\n";
  const char* type_line="Content-Type: text/html;charset=utf-8\n";
  const char* html="<h1>页面没找到</h1>";
  send(sock,first_line,strlen(first_line),0);
  send(sock,type_line,strlen(type_line),0);
  send(sock,black_line,strlen(black_line),0);//空行
  send(sock,html,strlen(html),0);
  return;
}
void PrintRequest(Request*req)
{
  printf("method: %s\n",req->method);
  printf("url_path %s\n",req->url_path);
  printf("query_string:%s\n",req->query_string);
  printf("content_length: %d\n",req->content_length);
  return ;
  
}
int IsDir(const char* file_path)
{
  struct stat st;
  int ret=stat(file_path,&st);
  if(ret<0)
  {
    return 0;
  }
  if(S_ISDIR(st.st_mode))
  {
    return 1;
  }
  return 0;
}
void HanderFilePath(const char*url_path,char file_path[])
{
  //给url_path加上前缀（http服务器的根目录）
  //url_path=> /index.html
  //file_path=> ./wwwroot/index.html
  sprintf(file_path,"./wwwroot%s",url_path);
  //url_path如果是目录，则追加一个index.html
  if (file_path[strlen(file_path)-1]=='/')
  {
    strcat(file_path,"index.html");
  }
  //例如url_path => /image
  if(IsDir(file_path))//判断是文件还是目录
  {
    strcat(file_path,"/index.html");
  }
  return;
}
ssize_t GetFileSize(const char* file_path)
{
  struct stat st;
  int ret=stat(file_path,&st);
  if(ret<0)
  {
    return 0;
  }
  return st.st_size;

}
int WriteStaticFile(int sock,const char *file_path)
{
  //打开文件
  int fd=open(file_path,O_RDONLY);
  if(fd<0)
  {
    //什么情况下打开失败？   1.文件描述符不够用时  2.文件不存在
    perror("open");
    return 404;
  }
  //把构造出来的HTTP响应写到sock中
  //a.写入首行
  const char * first_line="HTTP/1.1 200 OK\n";
  send(sock,first_line,strlen(first_line),0);
  //b写入header
  //const char* header_line="Content-Type: text/html;charset=utf-8\n";//可以让浏览器自动识别
  //send(sock,header_line,strlen(header_line));
  //c写入空行
  const char* black_line="\n";
  send(sock,black_line,strlen(black_line),0);
  //d写入body（文件内容）
  /*
     ssize_t file_size=GetFileSize(file_path);
     ssize_t i=0;
     for (int i = 0; i < file_size; ++i)
     {
     char c;
     read(fd,&c,1);
     send(sock,&c,1,0);
     }
     */
  sendfile(sock,fd,NULL,GetFileSize(file_path));
  //关闭文件
  close(fd);
  return 200;
}
int HandlerStaticFile(int sock,Request *req)
{
  //1.根据url_path获取到文件在服务器上的真实路劲
  char file_path[SIZE]={0};
  HanderFilePath(req->url_path,file_path);
  //2.读取文件，把文件的内容直接写到socket之中
  int err_code=WriteStaticFile(sock,file_path);
  return err_code;
}
int HanderCGIFather(int new_sock,int father_read,int father_write,int child_pid,Request*req)
{
  //如果是POST请求，就把body写入管道
  if(strcasecmp(req->method,"POST")==0)
  {
    int i=0;
    char c='\0';//放入
    for (; i < req->content_length; ++i)
    {
      read(new_sock,&c,1);
      write(father_write,&c,1);
      /* code */
    }
  }
  //2.构造HTTP响应
  const char * first_line="HTTP/1.1 200 OK\n";
  send(new_sock,first_line,strlen(first_line),0);
  const char*type_line="Content-Type: text/html;charset=utf-8\n";
  send(new_sock,type_line,strlen(type_line),0);
  const char *black_line="\n";
  send(new_sock,black_line,strlen(black_line),0);
  //3.循环的从管道中读取数据写入socket
  char c='\0';//放入
  while(read(father_read,&c,1)>0)
  {
    send(new_sock,&c,1,0);
  }
  //4.回收子进程的资源
  waitpid(child_pid,NULL,0);
    return 200;
}
int HanderCGIChild(int chiild_read,int chiild_write,Request *req)
{
  //1设置必要的环境变量
  char method_env[SIZE]={0};
  sprintf(method_env,"REQUEST_METHOD=%s",req->method);
  putenv(method_env);
  //还需要设置 QUERY_STRING 或者	CONTENT_LENGTN
  if(strcasecmp(req->method,"GET")==0)
  {
    char query_string_env[SIZE]={0};
    sprintf(query_string_env,"QUERY_STRING=%s",req->query_string);
    putenv(query_string_env);
  }
  else{
    char content_length_env[SIZE]={0};
    sprintf(content_length_env,"CONTENT_LENGTH=%d",req->content_length);
    putenv(content_length_env);
  }
  //2，把标准输入输出重定向到管道
  dup2(chiild_read,0);
  dup2(chiild_write,1);
  //3对子进程进行程序替换
  //根据 url_path: /cgi-bin/test
  //到 file_path: ./wwwroot/cgi-bin/test
  char file_path[SIZE]={0};
  HanderFilePath(req->url_path,file_path);
  execl(file_path,file_path,NULL);
  exit(1);
  return 200;
}
int HandlerCGI(int new_sock,Request*req)
{
  //1.创建一对匿名管道
  int fd1[2],fd2[2];
  int ret=pipe(fd1);
  int err_code=200;
  if(ret<0)
  {
    return 404;
  }
  ret=pipe(fd2);
  if (ret<0)
  {
    close(fd1[0]);
    close(fd1[1]);
    return 404;
  }
  //方便描述
  int father_read=fd1[0];
  int chiild_write=fd1[1];
  int father_write=fd2[1];
  int chiild_read=fd2[0];
  signal(SIGPIPE, SIG_IGN);
  //2.创建子进程
  ret=fork();
  //3.父子进程执行各自的不同逻辑
  if (ret>0)
  {
    //father
    //此处父进程优先关闭这俩个管道的文件描述符
    //是为了后续父进程从子进程这里读数据的时候，能够读到EOF，对于管道来说，所有的写端关闭。继续读
    //才有EOF。而此处的所有写端，一方面是父进程需要关闭，另一方面子进程也需要关闭
    //所以此处父进程先关闭不必要的写端后，后续子进程用完了
    //直接关闭，父进程也读到了EOF

    close(chiild_read);
    close(chiild_write);
    err_code=HanderCGIFather(new_sock,father_read,father_write,ret,req);
  }
  else if(ret==0)
  {
    close(father_write);
    close(father_read);
    err_code= HanderCGIChild(chiild_read,chiild_write,req);
    //chiild
  }
  else{
    perror("fork");
    err_code=404;
    goto END;
  }
END:
    close(fd1[0]);
    close(fd2[1]);
    close(fd2[0]);
    close(fd1[1]);
  
  return err_code;
  //4.收尾工作和错误处理
}
void HeadlerRequest(int new_sock)
{
  int err_code=200;
  //1.读取解析请求
  Request req;
  memset(&req,0,sizeof(req));
  //a.从socket中读出首行
  if(ReadLine(new_sock,req.first_line,sizeof(req.first_line))<0)
  {
    //读取失败
    err_code=404;
    goto END;

  }
  //b.首行读完解析首行，从中解析出url和method
  if(ParseFirstLine(req.first_line,&req.url,&req.method))
  {
    //失败处理
    err_code=404;
    goto END;
  }
  //c.解析url从中得出url_path,url_string
  if(ParseQueryString(req.url,&req.url_path,&req.query_string))
  {
    //
    err_code=404;
    goto END;
  }
  //d.处理head 丢弃了大部分header，只读取Content-length
  if(ParsHeader(new_sock,&req.content_length))
  {
    //
    err_code=404;
    goto END;
  }
PrintRequest(&req);
  //2.静态/动态方式生产页面,把生成从结果写到客户端上
  if (strcasecmp(req.method,"GET")==0   //不区分大小写
      &&req.query_string==NULL)
  {//a.如果请求是GET，并且没有query_string,呢么就返回静态页面
    err_code=HandlerStaticFile(new_sock,&req);
  }else if(strcasecmp(req.method,"GET")==0
      &&req.query_string!=NULL)
  {//b.如果请求是GET，并且有query_string,呢么就返回动态页面
    err_code=HandlerCGI(new_sock,&req);
  }else if(strcasecmp(req.method,"POST")==0)
  {//c.如果是post请求（一定是带参数的，参数是通过body来传递给服务器的，也是动态页面
    err_code=HandlerCGI(new_sock,&req);
  }else
  {
    //错误处理
    err_code=404;
    goto END;
  }


  //错误处理：直接返回一个404的HTTP响应
END:
  if(err_code!=200)
  {
    Hander404(new_sock);
  }
  close(new_sock);

  return;

}

void* ThreadEntry(void* arg)
{
  int64_t new_sock=(int64_t)arg;
  printf("%d\n\n\n",new_sock);
  HeadlerRequest(new_sock);//使用进行完成具体的处理请求过程。方便改成多进程或IO多路复用
  return NULL;
}



void HttpServerStart(const char *ip,short port)
{

  sockaddr_in addr;
  addr.sin_family=AF_INET;
  addr.sin_port=htons(port);
  addr.sin_addr.s_addr=inet_addr(ip);
  int listen_sock=socket(AF_INET,SOCK_STREAM,0);
  if(listen_sock<0)
  {
    perror("sock");
    return;
  }
  //重用 TIME_WAIT 链接
  int opt=1;//服务器主动断开
  setsockopt(listen_sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
    //防止断开后要等待一段时间后 端口才被使用。直接关闭并使用
  int ret=bind(listen_sock,(struct sockaddr*)&addr,sizeof(addr));
  if(ret<0)
  {
    perror("bind");
    return;
  }
  ret=listen(listen_sock,10);
  if(ret<0)
  {
    perror("listen");
    return;
  }
  printf("Serverinit ok\n");
  while(1)
  {
    struct sockaddr_in peer;
    socklen_t len=sizeof(peer);
    int64_t new_sock=accept(listen_sock,(struct sockaddr*)&peer,&len);
    if (new_sock<0)
    {
      perror("accept");
      /* code */
      continue;
    }
    //多线程实现
    pthread_t tid=0;
    pthread_create(&tid,NULL,ThreadEntry,(void*)new_sock);
    pthread_detach(tid);//自动回收

  }
}
int main(int argc,char *argv[])
{
  if(argc!=3)
  {
    printf("Usage ./http_server [ip][]port\n");
    return 1;
  }
  HttpServerStart(argv[1],atoi(argv[2]));
  return 0;
}
