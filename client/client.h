#ifndef __CLIENT_H__
#define __CLIENT_H__
#include <map>
#include <pthread.h>
#include <cstring>
#include <string>
#include "project.h"
#include "torrent.h"
#include "file.h"

#define LISTEN_PORT 10002

const string torrent_list_name = "torrent_list.txt";

class Client
{
private:
	pthread_t listenThread;
	Address personal_addr;
	Address tracker_addr;
	Address server_addr;
	string file_dir;
	vector<Torrent>  personal_tor_list;
	map<string, File*> launched_files;
	map<string, pthread_t> launch_file_list;
	/*
	// indicating what torrent files it has: "info hash code + check bit + file path",
	//check bit = 1: data fiule available, check bit = 0: invalid torrent.
	string  target_list; // check bit + piece number + client 1 + client 2+ ... client m
	string  downloading_torrent_path;// path of the torrent file being downloaded.
	*/	
	bool connect_Tracker(int &sock);
	bool connect_Server(int &sock);

public: 
	Client(char *config);
	~Client();
	// void *handler(void* arg);
	void start_torrent(const string& torrent_name);
	void stop_torrent(const string& torrent_name);
	
	bool publish_torrent(const string& filename);
	bool registeration(const string& hashcode, int piece_num, const string torName);
	/*
	** register at tracker.
	** Input: torrent file the client is registering; Address and port of tracker.
	** Return: ture, if registeration succeeds; false, otherwises.
	*/
	bool get_Tor_List_File();
	void show_tor_list();
	/*
	** get torrent file from web server
	** Return: true if torrent successfully receives; false otherwises.
	*/
	string chosen_Torrent(const string torrent_name); // return the info hash of chosen torrent file.
	bool download_Torrent(string  torrent_chosen_hash, string tor_name); 
	//return true if torrent file received. update personal_tor_list when finish, set check bit = 0;
	bool compare_Info_Hash(string torrent_chosen_hash, string tor_name); 
	// return true if calculated hash code of downloaded torrent file = hash code of chosen torrent file.

	//as downloader with other clients.
	bool request_reply(string file_name, const int &sock);
	/*
	** according the request from downloaders, choosing relavent pieces to them.
	*/

	//used by downloader	
	void handshake_reply(const int &sock);
	/*
	** reply the handshake request from prospective downloader.
	** If no file owning, drop connection; If file exists, send bitfield to downloader
	*/
	void* listen_peer(void*);
	void* listen_file(void*);
};

struct ClientPack{
	Client* client;
	int* sock;
};

struct BigPack {
	File* f;
	int port;
};

#endif
