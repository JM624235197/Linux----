#if 1
#pragma once
#include<iostream>
#include<sstream>
#include<fstream>
#include<vector>
#include<string>
#include<unordered_map>
#include<boost/filesystem.hpp>
#include<boost/algorithm/string.hpp>//splitͷ�ļ�
#include"httplib.h"

class FileUtil
{
public:
	//���ļ��ж�ȡ��������
	static bool Read(const std::string &name, std::string *body)
	{
		//�����ļ������Զ����Ʒ�ʽ���ļ�      
		std::ifstream fs(name, std::ios::binary);
		if (fs.is_open() == false){
			std::cout << "open file" << name << "failed!\n";
			return false;
		}
		//boost::filesystem::file_size()��ȡ�ļ���С
		int64_t fsize = boost::filesystem::file_size(name);
		//��body����ռ�����ļ�����
		body->resize(fsize);
		//��Ϊbody�Ǹ�ָ�룬��Ҫ�Ƚ�����
		fs.read(&(*body)[0], fsize);
		if (fs.good() == false){
			std::cout << "file" << name << "read data failed!\n";
			return false;
		}
		fs.close();
		return true;
	}

	//���ļ���д������
	static bool Write(const std::string &name, const std::string &body)
	{
		//�����--ofstreamĬ�ϴ��ļ���ʱ������ԭ�е�����
		//��ǰ�����Ǹ���д��
		std::ofstream ofs(name, std::ios::binary);
		if (ofs.is_open() == false){
			std::cout << "open file" << name << "failed!\n";
			return false;
		}
		ofs.write(&body[0], body.size());
		if (ofs.good() == false){
			std::cout << "file" << name << "write data failed!\n";
			return false;
		}
		ofs.close();
		return true;
	}
};

//�ͻ������ݹ���ģ�飺
//���ݹ���ģ��Ĺ��ܣ�1����Ŀ¼���ģ���ṩԭ��etag��Ϣ��2���ļ�����ģ�鱸���ļ��ɹ��󣬸�������
class DataManager
{
private:
	//�־û��洢�ļ�����
	std::string _store_file;
	//���ݵ��ļ���Ϣ
	std::unordered_map<std::string, std::string> _backup_list;
public:
	DataManager(const std::string &filename) :_store_file(filename)
	{}
	//����֮�����/����������Ϣ
	bool Insert(const std::string &key, const std::string &val)
	{
		_backup_list[key] = val;
		Storage();
		return true;
	}
	//��ȡ�ļ�ԭ��etag��Ϣ���бȶ�
	bool GetEtag(const std::string &key, std::string *val)
	{
		auto it = _backup_list.find(key);
		if (it == _backup_list.end())
		{
			return false;
		}
		*val = it->second;
		return true;
	}
	//��ʼ������ԭ������
	bool InitLoad()
	{
		//�����ݵĳ־û��洢�ļ��м�������
		//1������������ļ������ݶ�ȡ����
		std::string body;
		if (FileUtil::Read(_store_file, &body) == false){
			return false;
		}
		//2�������ַ�����������\r\n���зָ�
		//boost::split(vector,src,sep,flag);
		std::vector<std::string>list;
		boost::split(list, body, boost::is_any_of("\r\n"), boost::token_compress_off);
		//3��ÿһ�а��տո���зָ�--ǰ����key�������val
		for (auto i : list)
		{
			size_t pos = i.find(" ");
			if (pos == std::string::npos){
				continue;
			}
			std::string key = i.substr(0, pos);
			std::string val = i.substr(pos + 1);
			//4����key/val��ӵ�_file_list��
			Insert(key, val);
		}
			return true;
	}
	//ÿ�θ�������֮�󣬳־û��洢
		bool Storage()
		{
			//��_file_list�е����ݽ��г־û��洢
			//�����ݶ�����г־û��洢---���л�
			//filename eatg \r\n
			std::stringstream tmp;//��ȡһ��string������    
			auto it = _backup_list.begin();
			for (; it != _backup_list.end(); it++)
			{
				//������󣬽����е���Ϣ�����뵽string����
				//���л�-����ָ��������֯����
				tmp << it->first << " " << it->second << "\r\n";
			}
			//�����ݱ��ݵ��ļ���
			FileUtil::Write(_store_file, tmp.str());
		return true;
	}
};

//����һ��ȫ�ֱ���
//DataManager data_manage(STORE_FILE);

//Ŀ¼���ģ�飺
//���ܣ��ܹ���⵽����Щ�ļ���Ҫ���б���
class CloudClient
{
private:
	std::string _listen_dir;//���ģ��Ҫ��ص�Ŀ¼����
	DataManager data_manage;//���ݹ������
	std::string _srv_ip;//����˵ĵ�ַ
	uint16_t _srv_port;//����˵Ķ˿�
public:
	CloudClient(const std::string &filename, const std::string &store_file,
		const std::string &srv_ip, uint16_t	srv_port)
		:_listen_dir(filename), data_manage(store_file)
		,_srv_ip(srv_ip), _srv_port(srv_port)
		{}
	bool Start()
	{
		data_manage.InitLoad();//������ǰ���ݵ���Ϣ
		while (1)
		{
			std::vector<std::string>list;
			//��ȡ�����е���Ҫ���ݵ��ļ�����
			GetBackUpFileList(&list);
			for (int i = 0; i<list.size(); i++)
			{
				//���ļ���
				std::string name = list[i];
				//�ļ�·����
				std::string pathname = _listen_dir + name;
				std::cout << pathname << "is need to backup\n";
				//��ȡ�ļ����ݣ���Ϊ��������
				std::string body;
				FileUtil::Read(pathname, &body);
				//ʵ����Client����׼������HTTP�ϴ��ļ�����
				httplib::Client client(_srv_ip, _srv_port);
				std::string req_path = "/" + name;
				auto rsp = client.Put(req_path.c_str(), body, "application/octet-stream");
				if (rsp == NULL || (rsp != NULL&&rsp->status != 200))
				{
					//����ļ��ϴ�����ʧ����
					std::cout << pathname << "backup failed\n";
					continue;
				}
				//���ļ�����ʧ���ˣ���û�м�����������ļ���ǰ����Ϣ����Ϊ���´�ѭ��������������Ȼ���б���
				std::string etag;
				GetEtag(pathname, &etag);
				//���ݳɹ������/������Ϣ
				data_manage.Insert(name, etag);
				std::cout << pathname << "backup success\n";
			}
			Sleep(1000);//ÿ��1000ms�����¼��
		}
		return true;
	}
	//��ȡ��Ҫ���ݵ��ļ��б�--���Ŀ¼�������ļ��������ȡetagȻ����бȶ�
	bool GetBackUpFileList(std::vector<std::string> *list)
	{
		if (boost::filesystem::exists(_listen_dir) == false)
		{
			//��Ŀ¼�������򴴽�
			boost::filesystem::create_directory(_listen_dir);
		}
		//1������Ŀ¼��أ���ȡָ��Ŀ¼�������ļ�����
		boost::filesystem::directory_iterator begin(_listen_dir);
		boost::filesystem::directory_iterator end;
		for (; begin != end; begin++)
		{
			if (boost::filesystem::is_directory(begin->status()))
			{
				//Ŀ¼�ǲ�����Ҫ���б��ݵ�
				//��ǰ���ǲ�������㼶Ŀ¼���ݣ�����Ŀ¼ֱ��Խ��
				continue;
			}
			std::string pathname = begin->path().string();
			std::string name = begin->path().filename().string();
			std::string cur_etag;
			//2������ļ���������ǰetag
			GetEtag(pathname,&cur_etag);
			//3����ȡ�Ѿ����ݹ���etag��Ϣ
			std::string old_etag;
			data_manage.GetEtag(name, &old_etag);
			//4����data_manage�б����ԭ��etag���бȶ�
			//	��û���ҵ�ԭ��etag--���ļ���Ҫ����
			//	���ҵ�ԭ��etag�����ǵ�ǰetag��ԭ��etag����ȣ���Ҫ����
			//	���ҵ�ԭ��etag�������뵱ǰetag��ȣ�����Ҫ����
			if (cur_etag != old_etag)
			{
				//��ǰetag��ԭ��etag��ͬ����Ҫ���±���
				list->push_back(name);
			}
		}
		return true;
	}
	//����һ���ļ���etag��Ϣ
	bool GetEtag(const std::string &pathname, std::string *etag)
	{
		//etag���ļ���С--�ļ����һ���޸�ʱ��
		int64_t fsize=boost::filesystem::file_size(pathname);
		time_t mtime=boost::filesystem::last_write_time(pathname);//���һ���޸�ʱ��
		*etag = std::to_string(fsize) + "-" + std::to_string(mtime);
		return true;
	}
};

#endif
