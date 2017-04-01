#ifndef __UTIL_H__
#define __UTIL_H__
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <cstring>
#include <string>
#include <openssl/sha.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

int get_file_size(const char* filename);

string sha1sum(FILE* file, const int file_size=0);


#endif