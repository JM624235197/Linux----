#include"httplib.h"
#include<iostream>

void helloworld(const httplib::Request &req,httplib::Response &rsp)
{
  //rsp.status 状态码 rsp.headers 头部信息 rsp.body 正文
  //rsp.set_header(const char *key,const char *val)
  //rsp.body / rsp.set_content(const char*s,size_t n,const char *content_type)
  std::cout<<"method:"<<req.method<<std::endl;
  std::cout<<"path:"<<req.path<<std::endl;
  std::cout<<"body:"<<req.body<<std::endl;

  rsp.status=200;
  rsp.body="<html><body><h1>HelloWorld</h1><body></html>";
  rsp.set_header("Content-Type","text/html");
  rsp.set_header("Content-Length",std::to_string(rsp.body.size()));
  return;
}

int main()
{

  httplib::Server server;
  server.Get("/",helloworld);

  server.listen("0.0.0.0",9000);
  return 0 ;
}
