#include"http_server.h"

int ReadLine(int sock,char buf[],ssize_t size)
{
  char c='\0';
  ssize_t i=0;
  while(i<size-1&&c!='\n')
  {
    ssize_t read_size=recv(sock,&c,1,0);
    if(read_size<0)
    {
      return -1;
    }
    if(read_size==0)
    {
      return -1;
    }
    if(c=='\r')
    {
      recv(sock,&c,1,MSG_PEEK);
      if(c=='\n')
      {
        recv(sock,&c,1,0);
      }else{
        c='\n';
      }
    }
    buf[i++]=c;
  }
  buf[i]='\0';
  return i;
}
int Split(char input[],const char *split_char,char *output[],int output_size)
{
  char *pch;
  int i=0;
  char *tmp=NULL;
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
int ParseFirstLine(char first_line[],char **p_url,char **p_method)
{
  char *tok[10];
  int tok_size=Split(first_line," ",tok,10);
  if(tok_size!=3)
  {
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
  for(;*p!='\0';++p)
  {
    if(*p=='?')
    {
      *p='\0';
      *p_query_string=p+1;
      return 0;
    }
  }
  *p_query_string=NULL;
  return 0;
}
int ParsHeadeader(int sock,int *content_length)
{
  char buf[1024]={0};
  while(1)
  {
    ssize_t read_size=ReadLine(sock,buf,sizeof(buf));
    if(read_size<0)
    {
      return -1;
    }
    if(strcmp(buf,"\n")==0)
    {
      return 0;
    }
    if(content_length!=NULL&&strncmp(buf,"Content-Length: ",16)==0)
    {
      *content_length=atoi(buf+16);
    }
  }
  return 0;
}
void Hander404(int sock)
{
  const char *first_line="HTTP/1.1 404 Not Found\n";
  const char *type_line="Content-Type: text/html;charset=utf-8\n";
  const char *black_line="\n";
  const char *html="<h1>No Found!!!</h1>";
  send(sock,first_line,strlen(first_line),0);
  send(sock,type_line,strlen(type_line),0);
  send(sock,black_line,strlen(black_line),0);
  send(sock,html,strlen(html),0);
  return ;
}
void PrintRequest(Request *req)
{
  printf("method:%s\n",req->method);
 printf("url_path:%s\n",req->url_path);
 printf("query_string:%s\n",req->query_string);
 printf("content_length:%d\n",req->content_length);
}
void HeadlerRequest(int new_sock)
{
  int err_code=200;
  Request req;
  memset(&req,0,sizeof(req));
  if(ReadLine(new_sock,req.first_line,sizeof(req.first_line))<0)
  {
    err_code=404;
    goto END;  
  }
  if(ParseFirstLine(req.first_line,&req.url,&req.method))
  {
    err_code=404;
    goto END;
  }
  if(ParseQueryString(req.url,&req.url_path,&req.query_string))
  {
    err_code=404;
    goto END;
  }
  if(ParsHeadeader(new_sock,&req.content_length))
  {
    err_code=404;
    goto END;
  }
  PrintRequest(&req);


END:
  if(err_code!=200)
  {
    Hander404(new_sock);
  }
  return;
}

void* ThreadEntry(void *arg)
{
  int64_t new_sock=(int64_t)arg;
  HeadlerRequest(new_sock);
  return NULL;
}
void HttpServerStart(const char *ip,short port)
{
  struct sockaddr_in addr;
  addr.sin_addr.s_addr=inet_addr(ip);
  addr.sin_family=AF_INET;
  addr.sin_port=htons(port);
  int listen_sock=socket(AF_INET,SOCK_STREAM,0);
  if(listen_sock<0)
  {
    perror("sock");
    return;
  }
  int opt=1;
  setsockopt(listen_sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

  int ret=bind(listen_sock,(struct sockaddr*)&addr,sizeof(addr));
  if(ret<0)
  {
    perror("bind");
    return;
  }
  ret=listen(listen_sock,5);
  if(ret<0)
  {
    perror("listen");
    return;
  }
  printf("Service Init!!!\n");
  while(1)
  {
    struct sockaddr_in peer;
    socklen_t len=sizeof(peer);
    int64_t new_sock=accept(listen_sock,(struct sockaddr*)&peer,&len);
    if(new_sock<0)
    {
      perror("accept");
      continue; 
    }

    pthread_t tid=0;
    pthread_create(&tid,NULL,ThreadEntry,(void*)new_sock);
    pthread_detach(tid);

  }

}
int main(int argc,char *argv[])
{
  if(argc!=3)
  {
    printf("!!![ip][port]\n");
    return 1;
  }
  HttpServerStart(argv[1],atoi(argv[2]));
}
