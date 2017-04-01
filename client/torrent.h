#ifndef __TOR_H__
#define __TOR_H__
#include <cstring>
#include <string>
#include "project.h"

class Torrent
{
private:
	string tor_Path;
	string file_Path;
	string hashcode;
	string tor_Name;
	string file_Name;
	int total_Piece;
	int file_size;
	vector<string> piece_hash;
	bool connect_server(Address server_addr, int& sock);
	//bool connect_Target(Client target);
public:
	Torrent();
	Torrent(const string file_dir, const string tor_name);
	~Torrent();

	bool torrent_creation(const string upload_dir, const string file_name);
	/*
	** used to create torrent file
	** Input: names for the file and corresponding torrent file.
	** Return: ture, if file successfully creates; false, others.
	*/
	bool publish_torrent(Address server); // first connects to server;
	/*
	** after creating torrent file, send the torrent file to web server.
	** Input: name of torrent file; IP and port of server; boundary time for publishing.
	** Return: ture, if publishes succeeds; false, others.
	*/
	string get_Hash_Code();
	//bool set_Tor_Path(const string file_path);
	string get_file_name();
	string get_tor_name();
	string get_file_path();
	int get_total_piece();
	int get_size();
	// return the hash code for corresponding piece.
	string resolve_tor(int piece_number);

};
#endif