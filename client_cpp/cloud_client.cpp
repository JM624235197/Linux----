#include"cloud_client.hpp"
#include<stdlib.h>

#define STORE_FILE "./list.backup"
#define LISTEN_DIR "./backup/"
#define SERVER_IP "192.186.56.101"
#define SERVER_PORT 9000

int main()
{
	CloudClient client(LISTEN_DIR,STORE_FILE,SERVER_IP,SERVER_PORT);
	client.Start();
	return 0;
	system("pause");
}