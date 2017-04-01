#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <iostream>
#include <sstream>
#include <netdb.h>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <cassert>

#define BUF_SIZE 1024
#define MAX_CONS 5
#ifdef __APPLE__
    #define MSG_NOSIGNAL 0
#endif
using namespace std;


int receive_len(int sock);

int receive_data(int sock, char* recv, int size, FILE* file=NULL);

int receive_file(int sock, FILE* file);

/* usage: char* recv;
 *        receive_msg(sock, &recv);
 */
int receive_msg(int sock, char** recv);

int send_data(int sock, const char* data, int size, FILE* file=NULL);

int init_socket(const char* addr=NULL, int port=0, bool quiet=false);


#endif /* _SOCKET_H_ */