#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <cstring>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <sstream>
#include "server_lib.h"
#include "../socket.h"
#include "../util.h"

using namespace std;

Server :: Server(char *config) {
	ifstream iConfigFile;
	iConfigFile.open(config);
	string junk, message;
	iConfigFile >> junk >> message;
	port = atoi(message.c_str());
	iConfigFile >> junk >> message;
	torList = message;
	iConfigFile >> junk >> message;
	torPath = message;
	cout << port << endl << torList << endl << torPath << endl;
}

void *callHandler(void *type) {
	cout << "thread created" << endl;
	ServerPack *sp = static_cast<ServerPack *>(type);
	return (sp -> server -> handler(sp -> sock));
}

void *Server :: handler(void *socketDesc) {
	int sock = *(int*)socketDesc;
	char *msg;
	receive_msg(sock, &msg);
	string msgStr(msg);
	cout << msgStr << endl;
	if (msgStr == "torrent_list") {
		cout << "Sending torrent list" << endl;
		send_data(sock, confirm, strlen(confirm));
		if (sendTorList(sock))
			cout << "Torrent list sent" << endl;
		else
			cout << "Torrent list sent failed!" << endl;
	} else if (msgStr == "download_torrent") {
		send_data(sock, confirm, strlen(confirm));
		char *fileHash;
		cout << "Requesting file hash " << endl;
		receive_msg(sock, &fileHash);
		cout << "Sending torrent " << fileHash << endl;
		sendTor(sock, fileHash);
		cout << "Torrent sent" << endl;
	} else if (msgStr == "publish") {
		send_data(sock, confirm, strlen(confirm));
		cout << "Receiving torrent file" << endl;
		receiveTorrent(sock);
		cout << "Torrent file received" << endl;
	}
	delete []msg;
	free(socketDesc);
	pthread_detach(pthread_self());
	cout << "thread return" << endl;
	return 0;
}

bool Server :: createServer() {
	int client_sock, c, *new_sock;
	struct sockaddr_in client;
	int socketDesc = init_socket(NULL, port);
	c = sizeof(sockaddr_in);

	while((client_sock = accept(socketDesc, (sockaddr *)&client, (socklen_t*)&c))) {
		cout << endl << "Connection accepted" << endl;
		cout << "The client is " << endl;
		cout << inet_ntoa(client.sin_addr) << endl;
        	cout << ntohs(client.sin_port) << endl;

		pthread_t newThread;
		new_sock = new int;
		ServerPack *sp = new ServerPack;
		sp -> server = this;
		sp -> sock = new_sock;
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

bool Server :: sendTorList(int sock) {
	FILE *fp = fopen(torList.c_str(), "r");
	if (fp == NULL) return false;
	int size = get_file_size(torList.c_str());
	send_data(sock, NULL, size, fp);
	send_data(sock, endList, strlen(endList));

	fclose(fp);
	return true;
}

bool Server :: sendTor(int sock, string reqTorrent) {
	string str = torPath + reqTorrent;
	FILE *fp = fopen(str.c_str(), "r");
	cout << str << endl;
	if (!fp) {
		cout << "fopen wrong" << endl;
		return false;
	}
	int size = get_file_size(str.c_str());
	send_data(sock, NULL, size, fp);
	send_data(sock, endTorrent, strlen(endTorrent));

	fclose(fp);
	return true;
}

bool Server :: receiveTorrent(int sock) {
	char *fileHash;
	receive_msg(sock, &fileHash);
	cout << "hash: " << fileHash << endl;

	string tor_name(fileHash);
	tor_name = torPath + tor_name;
	FILE *torFile;
	torFile = fopen(tor_name.c_str(), "w+");

	char *fileName;
	receive_msg(sock, &fileName);
	cout << "fileName: " << fileName << endl;
	receive_file(sock, torFile);

	int size = get_file_size(tor_name.c_str());
	string sha = sha1sum(torFile, size);
	send_data(sock, sha.c_str(), sha.length());

	if (!updateTorList(fileName, fileHash)) {
		cerr << "Update file list failed" << endl;
		return false;
	}

	fclose(torFile);
	delete[] fileHash;
	delete[] fileName;
	return true;
}

bool Server :: updateTorList(char *fileName, char *fileHash) {
	ifstream itorListFile(torList.c_str());
	string line;
	while (getline(itorListFile, line)) {
		istringstream istream;
		istream.str(line);
		string hash;
		istream >> hash;
		if (hash == string(fileHash)) {
			itorListFile.close();
			return true;
		}
	}
	itorListFile.close();
	ofstream torListFile;
	torListFile.open(torList.c_str(), ofstream::app);
	torListFile << fileHash << " " << fileName << endl;
	torListFile.close();
	return true;
}

Server :: ~Server() {}