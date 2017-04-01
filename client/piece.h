#ifndef __PIECE_H__
#define __PIECE_H__
#include "project.h"

class Piece
{
private:
	string hash_code;
	int length;
	int piece_number;
	int status;// 1: owned.
public:
	Piece();
	~Piece();
	string get_Hash_Code();
	int get_length();
	int get_piece_number();
	int get_status();
	void set_Piece(Piece new_piece);
	void update_status();
};
#endif