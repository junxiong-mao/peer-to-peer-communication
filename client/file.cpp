#include <stdio.h>
#include <fstream>
#include <iostream>
#include <cstring>
#include <string>
#include <iomanip>

#include "file.h"
#include "../socket.h"
#include "../util.h"

bool File::connect_uploader(Address* uploader, int& sock){
	sock = init_socket(uploader -> ip.c_str(), atoi(uploader -> port.c_str()), true);
	return (sock >= 0);
}

bool File::check_Correctness_Of_Piece(string piece_hash, int piece_number){
	return (piece_hash == hashcode[piece_number]);
}

bool File::connect_tracker(int& sock, int& port) {
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
	    perror("Could not create socket");
	    return false;
	}
	// connect
	sockaddr_in tracker;
	tracker.sin_family = AF_INET;
	tracker.sin_addr.s_addr = inet_addr(tracker_addr.ip.c_str());
	tracker.sin_port = htons(atoi(tracker_addr.port.c_str()));
	int yes = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0){
	    perror("set options failed. Error");
	    return false;
	}
	if (connect(sock, (struct sockaddr *)&tracker , sizeof(tracker)) < 0) {
	    perror("connect failed. Error");
	    return false;
	}
	// puts("Connected\n");
	// get sock information
	sockaddr_in client;
	int length = sizeof(client);
	if (getsockname(sock, (struct sockaddr*) &client, (socklen_t*) &length) < 0){
	    puts("get socket name failed. Error");
	    return -1;
	}
	// cout << "local port is " << ntohs(client.sin_port) << endl;
	port = ntohs(client.sin_port);
	return (sock >= 0);
}

File::File() {};
File::~File() {};
File::File(Torrent* _torrent){
	//file_Path = torrent.get_tor_path();
	file_name = _torrent->get_file_name();
	num_of_piece = _torrent->get_total_piece();
	file_path = _torrent -> get_file_path();
	file_size = _torrent->get_size();
	torrent = _torrent;
	for (int i = 0; i < num_of_piece; i++) {
		hashcode.push_back(_torrent->resolve_tor(i));
	}
}

bool File::rqst_peer_list(int& port, bool downloaded){
	initialize_piece_status();
	int sock;
	if(!connect_tracker(sock, port)) {
		return false;
	}
	string message = "peer_list";
	send_data(sock, message.c_str(), message.size());
	char *msg;
	receive_msg(sock, &msg);
	if(strcmp(msg,"confirm") != 0) {
		return false;
	}
	string hash_torrent = torrent->get_Hash_Code();
	if (downloaded) {
		string bitField = string(torrent -> get_total_piece(), '1');
		string fileInfo = hash_torrent + " " + bitField + " " + torrent -> get_tor_name();
		send_data(sock, fileInfo.c_str(), fileInfo.size());
		return true;
	} else {
		string bitField = string(torrent -> get_total_piece(), '0');
		string fileInfo = hash_torrent + " " + bitField + " " + torrent -> get_tor_name();
		send_data(sock, fileInfo.c_str(), fileInfo.size());
	}
	
	receive_msg(sock, &msg);
	while (strcmp(msg, "end_list")){
		Address* new_peer = new Address;
		string buf(msg);
		istringstream iPeerInfo;
		iPeerInfo.str(buf);
		string bitField;
		iPeerInfo >> bitField >> new_peer -> ip >> new_peer -> port;
		peer_status[new_peer->str()] = false;
		peer_list.push_back(new_peer);
		for (unsigned int i = 0; i < bitField.length(); i++) {
			if (bitField[i] == '1') {
				piece_status[i] -> target_client.push_back(*new_peer);
			}
		}
		receive_msg(sock, &msg);
	}
	cout << "End list" << endl;
	delete[] msg;
	return true;
}

void File::initialize_piece_status() {
	cout << "initializing " << num_of_piece << endl;
	for (int i = 0; i < num_of_piece; i++) {
		Target_Piece *newPiece = new Target_Piece;
		newPiece -> check_bit = 0;
		newPiece -> hash_code = hashcode[i];
		newPiece -> piece_number = i;
		newPiece -> target_client.clear();

		char buf[10];
		sprintf(buf, "%d", i);
		string num = buf;
		FILE *fp_seg;
		string seg_name = file_path + "_" + num;
		fp_seg = fopen(seg_name.c_str(), "rb");
		if (fp_seg) {
			string sha1 = sha1sum(fp_seg, get_file_size(seg_name.c_str()));
			if (check_Correctness_Of_Piece(sha1, i)) {
				cout << seg_name << " has been downloaded." << endl;
				newPiece -> check_bit = 1;
			}
			fclose(fp_seg);
		}
		piece_status.push_back(newPiece);
	}
}

int File::send_request(int piece_number, int sock){
	// cout << "sending request " << piece_number << endl;
	char buf[10];
	int status;
	sprintf(buf, "%d", piece_number);
	string num = buf;
	status = send_data(sock, num.c_str(), num.size()); // request piece[piece_number]
	FILE *fp;
	string seg_name = file_path + "_" + num;
	fp = fopen(seg_name.c_str(), "wb");
	status = receive_file(sock, fp);
	fclose(fp);
	return status;
}

void File::update_piece_status(int piece_num, bool add, bool drop, Address peer){
	if(drop == 0) {
		if(add == 0)
			piece_status[piece_num] -> check_bit = 1;// 1 indeicates downloaded.
		else 
			piece_status[piece_num] -> target_client.push_back(peer); // add one more peer to piece[piece_num]
	} else {
		int size = piece_status[piece_num] -> target_client.size();
		for (int i = 0; i < size; i++){
			if (piece_status[piece_num] -> target_client[i].ip == peer.ip &&
			piece_status[piece_num] -> target_client[i].port == peer.port)				
			{
				piece_status[piece_num] -> target_client.erase(piece_status[piece_num] -> target_client.begin() + i);
				break;
			}
		}
	}
}

void File::have(int piece_num, string tor_name, int listenPort) {
	int sock, port;
	if (!connect_tracker(sock, port)) {
		return;
	}
	if (send_data(sock, "broadcast", 9) < 0) {
		return;
	}
	char *msg;
	receive_msg(sock, &msg);
	if (strcmp(msg, "confirm")) {
		return;
	}
	string cat;
	ostringstream ostream;
	ostream << piece_num << " " << tor_name << " " <<listenPort;
	cat = ostream.str();
	send_data(sock, cat.c_str(), cat.size());
	return;
}

void File::rearrange_Pieces() {}

bool File::downloaded() {
	FILE* fp;
	fp = fopen(file_path.c_str(), "rb");
	if (fp == NULL) {
		return false;
	}
	fclose(fp);
	// cout << "filepath: " << file_path << endl;
	int size = get_file_size(file_path.c_str());
	// cout << "size: " << size << endl;
	return size == file_size;
}

void* callFileHandler(void* type) {
	FilePack* file_pack = (FilePack*)type;
	return file_pack->file->hand_shake(file_pack->num, file_pack -> listenPort);
}

void* File::file_handler(int listenPort) {
	cout << "We have " << peer_list.size() << " peers." << endl;
	pthread_t *peerThread = new pthread_t[piece_status.size()];
	for (unsigned int i = 0; i < piece_status.size(); i++) {
		FilePack* file_pack = new FilePack;
		file_pack->file = this;
		int* num = new int;
		*num = i;
		file_pack->num = num;
		int* lp = new int(listenPort);
		file_pack -> listenPort = lp;
		pthread_create(&peerThread[i], NULL, callFileHandler, (void*)file_pack);
	}
	for (unsigned int i = 0; i < piece_status.size(); i++) {
		pthread_join(peerThread[i], NULL);
	}
	cout << "all peers tried" << endl;

    // combine file
	if (pieces_downloaded()) {
		cout << "start combining " << file_name << endl;
		FILE* fp;
		cout << "file path is " << torrent->get_file_path() << endl;
		fp = fopen(torrent->get_file_path().c_str(), "wb");
		for (unsigned int i = 0; i < piece_status.size(); i++) {
			ostringstream ostream;
			ostream << torrent->get_file_path() << "_" << i;
			string temp = ostream.str();
			FILE* segment;
			segment = fopen(temp.c_str(), "rb");
			int size;
			char* buf = new char[BUF_SIZE];
			while((size = fread(buf, 1, BUF_SIZE, segment))>0) {
				fwrite(buf, sizeof(char), size, fp);
				memset(buf,0, BUF_SIZE);
			}
			fclose(segment);
		}
		fclose(fp);
		cout << "combined file successfully" << endl;
	}
	return NULL;
}

void* File::hand_shake(void* arg, int* listenPort){
	// cout << "handshake" << endl;
	int sock;
	int num = *(int*)arg;
	while (!piece_status[num] -> check_bit) {
		for (unsigned int i = 0; i < piece_status[num] -> target_client.size(); i++) {
			if (peer_status[piece_status[num] -> target_client[i].str()]) continue;
			peer_status[piece_status[num] -> target_client[i].str()] = true;
			if (!connect_uploader(&(piece_status[num] -> target_client[i]), sock)) {
				// cout << "cannot connect with " << piece_status[num] -> target_client[i].port << endl;
				continue;
			}
			//cout << "connected with " << piece_status[num] -> target_client[i].ip << ' ' << piece_status[num] -> target_client[i].port << endl;
			string torrent_name = torrent->get_tor_name();
			if (send_data(sock, "handshake", 9) < 0) {
				continue;
			}
			char *confirm;
			receive_msg(sock, &confirm);
			if (strcmp(confirm, "confirm")) {
				continue;
			}
			if (send_data(sock, torrent_name.c_str(), torrent_name.size()) < 0) {
				continue;
			}
			if (send_request(num, sock) >= 0) {
				char buf[10];
				sprintf(buf, "%d", num);
				string number = buf;
				FILE *fp;
				string seg_name = file_path + "_" + number;
				//cout << seg_name << endl;
				fp = fopen(seg_name.c_str(), "rb");
				if (!check_Correctness_Of_Piece(sha1sum(fp, get_file_size(seg_name.c_str())), num) ) {
					fclose(fp);
					continue;
				}
				cout << num << " segment received from ";
				cout << piece_status[num] -> target_client[i].ip << ": " << piece_status[num] -> target_client[i].port << endl;
				piece_status[num]->check_bit = true;
				send_data(sock, "end_request", 11);
				peer_status[piece_status[num] -> target_client[i].str()] = false;
				have(num, torrent -> get_tor_name(), *listenPort);
				break;
			} else {
			}
		}
	}
	

	//delete[] reply;
	return NULL;
}

bool File::pieces_downloaded() {
	for (unsigned int i = 0; i < piece_status.size(); i++)
		if (!piece_status[i] -> check_bit) return false;
	return true;
}