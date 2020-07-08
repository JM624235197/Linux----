//boost库实现目录迭代
//scandir:封装目录浏览的功能
#include<iostream>
#include<string>
#include<boost/filesystem.hpp>

int main()
{
  std::string dir="./";
  //定义目录迭代器
  boost::filesystem::directory_iterator begin(dir);
  boost::filesystem::directory_iterator end;

  for(;begin!=end;begin++)
  {  
    //开始迭代获取目录下的文件名称
    //begin->path() 文件名的boost::filesystem::path对象
    //带路径的文件名
    std::string pathname=begin->path().string();
    //纯文件名
    std::string name=begin->path().filename().string();
    //过滤目录，只考虑普通文件
    //begin->status() 文件的属性信息
    //boost::filesystem::is_diretcory() 判断文件是否是目录
    if(boost::filesystem::is_directory(begin->status()))
    {
      std::cout<<pathname<<"is diretcory\n";
      continue;
    }
    std::cout<<name<<std::endl;
    std::cout<<pathname<<std::endl;
    std::cout<<name<<std::endl;
  }

  return 0;
}
