#ifndef __PROJECT_H__
#define __PROJECT_H__
#include <string>
#include <vector>

#define FILE_SIZE 4000000
using namespace std;

struct Address
{
	string ip;
	string port;
	string str() {
		return ip + " " + port;
	}
};


struct Target_Piece
{
	bool check_bit;
	int piece_number;
	string hash_code;
	vector<Address> target_client;
};

#endif
