#ifndef __TRACKER_LIB__
#define __TRACKER_LIB__

#include <string>
#include "../client/project.h"
#include <vector>

using namespace std;

void *callHandler(void *type);

const int BUFSIZE = 1000;
const int MSGLEN = 40;
const char confirm[] = "confirm";

struct ConnectClient {
	Address clientAddr;
	string torName;
};

class Tracker
{
private:
	int port;
	string peerList;
	bool updatePeerList(string peerInfoStr, char* client_addr, int client_port);
	/*
	** update peer list, if any new registeration, add it to a file named peer list.
	** Input: the reference of peer list file; client_info = IP + Port + SHA_1
	*/ 
	void sendPeerInfo(int sock, string fileHash);
	/*
	** send one downloader needed peerlist from overall Peerlist
	** Input: overall PeerList
	** Return: true, send successfully, false otherwise.
	*/
	void broadcast(string pieceInfo);

	vector<ConnectClient> clientVector;
public:
	Tracker(char *config);
	bool createServer();
	void *handler(void *socketDesc, char* client_addr, int client_port);
	~Tracker();
};

struct TrackerPack {
	Tracker *tracker;
	int *sock;
	char* client_addr;
	int client_port;
};

#endif