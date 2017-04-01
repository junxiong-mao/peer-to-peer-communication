#ifndef __SERVER_H__
#define __SERVER_H__
#include <string>
using namespace std;


const int BUFSIZE = 1000;
const int MSGLEN = 40;
const char confirm[] = "confirm";
const char endList[] = "end_list";
const char endTorrent[] = "end_torrent";

class Server
{
private:
	int port;
	string torList;
	string torPath;
	bool receiveTorrent(int sock);
	bool sendTorList(int sock);
	/*
	** send torrent list to downloader
	** Input: the file containing all names of torrent files.
	** Return: ture if successfully send, false otherwise;
	*/
	bool updateTorList(char *fileName, char *fileHash);
	bool sendTor(int sock, string reqTorrent);
	/*
	** send requested torrent file to downloader.
	** Input: the name of requested torrent file.
	** Return: ture, if send successfully, false otherwise. 
	*/
public:
	Server(char *config);
	void *handler(void *socket);
	bool createServer();// undermined!!
	~Server();
};

struct ServerPack {
	Server *server;
	int *sock;
};

void *callHandler(void *type);


#endif