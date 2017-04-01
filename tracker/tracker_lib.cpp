#include "tracker_lib.h"
#include "../socket.h"
#include <pthread.h>
#include <fcntl.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>
#include "../client/project.h"

using namespace std;

Tracker :: Tracker(char *config) {
	ifstream iConfigFile;
	iConfigFile.open(config);
	string junk, message;
	iConfigFile >> junk >> message;
	port = atoi(message.c_str());
	iConfigFile >> junk >> message;
	peerList = message;
	cout << "config is " << port << " " << peerList << endl << endl;
}

void *callHandler(void *type) {
	TrackerPack *sp = static_cast<TrackerPack *>(type);
	return (sp -> tracker -> handler(sp -> sock, sp -> client_addr, sp -> client_port));
}

void *Tracker :: handler(void *socketDesc, char* client_addr, int client_port) {
	int sock = *(int*)socketDesc;
	char* msg;
	receive_msg(sock, &msg);
	string msgStr(msg);
	delete[] msg;
	if (msgStr == "register") {
		send_data(sock, confirm, strlen(confirm));
		char *msg;
		receive_msg(sock, &msg);
		cout << "Updating peer list" << endl;
		updatePeerList(msg, client_addr, client_port);
		cout << "Update succeed" << endl;
		ostringstream ostream;
		ostream << client_addr << " " << client_port;
		string client_info;
		client_info = ostream.str();
		send_data(sock, client_info.c_str(), client_info.size());
		delete []msg;
	} else if (msgStr == "peer_list") {
		Address addr;
		ostringstream ostream;
		addr.ip = client_addr;
		ostream << client_port;
		addr.port = ostream.str();
		
		ConnectClient clientStruct;
		clientStruct.clientAddr = addr;

		send_data(sock, confirm, strlen(confirm));
		char *peerInfo;
		receive_msg(sock, &peerInfo);
		string peerInfoStr(peerInfo);
		istringstream istream(peerInfoStr);

		string hash, bitField, torName;
		istream >> hash >> bitField >> torName;
		clientStruct.torName = torName;
		clientVector.push_back(clientStruct);
		cout << "received hash: " << hash;
		cout << "Sending peer information" << endl;
		sendPeerInfo(sock, hash);
		if (!updatePeerList(peerInfoStr, client_addr, client_port)) {
			cout << "Peer info already in the list" << endl;
		}
		send_data(sock, confirm, strlen(confirm));
		delete[] peerInfo;
		cout << "Sent peer information" << endl;
	} else if (msgStr == "broadcast") {
		cout << "broadcast from client " << client_addr << " " << client_port << endl;
		send_data(sock, confirm, strlen(confirm));
		char *pieceInfo;
		receive_msg(sock, &pieceInfo);
		string clientPieceInfo;
		ostringstream clientPieceStream;
		clientPieceStream << pieceInfo << " " << client_addr;
		clientPieceInfo = clientPieceStream.str();
		broadcast(clientPieceInfo);
	}
	free(socketDesc);
	pthread_detach(pthread_self());
	return 0;
}

void Tracker::broadcast(string pieceInfo) {
	//pieceNum torName listenPort ip
	cout << "in broadcast function" << endl;
	istringstream pieceInfostream(pieceInfo);
	string pieceNum;
	string torName;
	int listenPort;

	pieceInfostream >> pieceNum >> torName >> listenPort;
	for (unsigned int i = 0; i < clientVector.size(); i++) {
		cout << "vector " << i << " ip: " << clientVector[i].clientAddr.ip << " port: " << clientVector[i].clientAddr.port << " piece number " << pieceNum << endl;
	}
	for (unsigned int i = 0; i < clientVector.size(); i++) {
		if (clientVector[i].torName == torName) {
			int port;
			istringstream portStream(clientVector[i].clientAddr.port);
			portStream >> port;
			cout << "this is  " << i << " ip: " << clientVector[i].clientAddr.ip << " port: " << port << endl;
			int sock;
			if ((sock = init_socket(clientVector[i].clientAddr.ip.c_str(), port, true)) < 0) {
				cout << "not connected Broadcast!!" << endl;
				continue;
			}
			if (send_data(sock, "broadcast", 9) < 0) {
				cout << "piece " << pieceNum << " send broadcast failed" << endl;
				continue;
			}
			char *confirm;
			receive_msg(sock, &confirm);
			if (strcmp(confirm, "confirm")) {
				cout << "piece " << pieceNum << " not confirm" << endl;
				continue;
			}
			if (send_data(sock, pieceInfo.c_str(), pieceInfo.length()) < 0) {
				cout << "piece " << pieceNum << " send pieceInfo failed" << endl;
				continue;
			}
			cout << "Broadcast succeed " << clientVector[i].clientAddr.ip << " " << clientVector[i].clientAddr.port << " for piece " << pieceNum << endl;
		}
	}
}

bool Tracker::createServer() {
	int client_sock, c, *new_sock;
	struct sockaddr_in client;
	int socketDesc = init_socket(NULL, port);
	c = sizeof(sockaddr_in);

	while((client_sock = accept(socketDesc, (sockaddr *)&client, (socklen_t*)&c))) {
		cout << "Connection accepted" << endl;
		cout << "The client is " << inet_ntoa(client.sin_addr) << ":" << ntohs(client.sin_port) << endl;
		char* client_addr = inet_ntoa(client.sin_addr);
		int client_port = ntohs(client.sin_port);
		

		pthread_t newThread;
		new_sock = new int;
		TrackerPack *sp = new TrackerPack;
		sp -> tracker = this;
		sp -> sock = new_sock;
		sp -> client_addr = client_addr;	
		sp -> client_port = client_port;
		*new_sock = client_sock;
		if(pthread_create(&newThread, NULL, callHandler, static_cast<void *>(sp)) < 0) {
			cerr << "could not create thread" << endl;
			return false;
		}
		cout << "Handler assigned" << endl;
	}
	
	if (client_sock < 0) {
		cerr << "accept failed" << endl;
		return false;
	}
	return true;
}

bool Tracker :: updatePeerList(string peerInfoStr, char* client_addr, int client_port) {
	ifstream iPeerFile(peerList.c_str());
	string line;
	string convertAddr(client_addr);
	while (getline(iPeerFile, line)) {
		string hashStr, junk, ipStr;
		int portStr;
		istringstream istream(line);
		istream >> hashStr >> junk >> junk >> ipStr >> portStr;
		if (hashStr == peerInfoStr && ipStr == convertAddr && portStr == client_port) {
			return false;
		}
	}
	iPeerFile.close();
	ofstream oPeerFile(peerList.c_str(), ofstream :: app);
	oPeerFile << peerInfoStr << ' ' << client_addr << ' ' << client_port << endl;
	oPeerFile.close();
	return true;
}
/*
** update peer list, if any new registeration, add it to a file named peer list.
** Input: the reference of peer list file; client_info = IP + bitfield + Port + SHA_1
*/ 
void Tracker :: sendPeerInfo(int sock, string fileHash) {
	ifstream iPeerFile(peerList.c_str());
	string ip;
	string port;
	string bitField;
	string hash;
	string torName;
	while (iPeerFile >> hash >> bitField >> torName >> ip >> port) {
		if (hash == fileHash) {
			string concant = bitField + " " + ip + " " + port;
			send_data(sock, concant.c_str(), concant.length());
		}
	}
	send_data(sock, "end_list", 8);
	iPeerFile.close();
}

Tracker :: ~Tracker() {}