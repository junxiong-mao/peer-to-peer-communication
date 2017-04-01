#include <stdio.h>
#include <fstream>
#include <iostream>
#include <cstring>
#include <string>
#include <iomanip>
#include "piece.h"

Piece::Piece():length(0),piece_number(0){};
Piece::~Piece() {};
string Piece::get_Hash_Code(){
	return hash_code;
}
int Piece::get_length(){
	return length;
}
int Piece::get_piece_number(){
	return piece_number;
}
int Piece::get_status(){
	return status;
}
void Piece::set_Piece(Piece new_piece){
	hash_code = new_piece.get_Hash_Code();
	length = new_piece.get_length();
	piece_number = new_piece.get_piece_number();
	status = new_piece.get_status();
}
void Piece::update_status(){
	if (status != 1) status = 1;
}