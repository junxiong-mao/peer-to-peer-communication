#ifndef __FILE_H__
#define __FILE_H__
#include "project.h"
#include "piece.h"
#include "torrent.h"
#include <map>

class File
{
private:
	string file_path;
	int num_of_piece;
	int file_size;
	vector<string> hashcode;
	map<string, bool> peer_status;
	/* a sub list of peer_list indicating who owns needed data file.
	** stores the address of allow_list file
	*/
	bool connect_uploader(Address* uploader, int& sock);
	bool connect_tracker(int& sock, int& port);
	bool check_Correctness_Of_Piece(string piece_hash, int piece_number);// if correct, send "Have", delete if wrong.
	// downloading file and rearrange are not finished.
	void rearrange_Pieces();
public:
	Address tracker_addr;
	string file_name;
	Torrent* torrent;
	vector<Target_Piece*> piece_status;
	vector<Address*> peer_list;
	File();
	File(Torrent* _torrent);
	~File();
	bool rqst_peer_list(int& port, bool downloaded);
	bool reg_peer_list(int& port);
	void initialize_piece_status();
	//bool handshake(Address target_client, int& sock);
	/*
	** create handshake with uploader. 
	** Input: bitfield array to store blocks uploader owning
	** Return: ture if bitfield correctly received; otherwise, drop connection and return false.
	*/
	int send_request(int piece_number, int sock);
	/* 
	** send request to uploader with request message
	** Input: Message <length prefix><message id><piece index><begin><block>
	** Return: true if request_reply successfully received; false if timeout.
	*/	
	void update_piece_status(int piece_num, bool add, bool drop, Address peer); 
	// if drop = 1; for D_piece_num, dropPeer does not support downloading
	// if drop = 0; it's not used for dropping but indicating D_piece_num has been downloaded.

	void update_Peer_List(Address new_peer);
	void have(int piece_num, string tor_namem, int listenPort); // broadcasting to tell other peers one piece he just owns.
	bool downloaded();
	bool pieces_downloaded();
	void* file_handler(int listenPort);
	void* download_file();
	void* hand_shake(void*, int*);
};

struct FilePack {
	File* file;
	int* num;
	int* listenPort;
};

#endif
