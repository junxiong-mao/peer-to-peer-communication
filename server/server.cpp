#include <iostream>
#include "server_lib.h"
using namespace std;

int main(int argc, char *argv[]) {
	Server s(argv[1]);
	s.createServer();
	return 0;
}