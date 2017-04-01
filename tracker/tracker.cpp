#include <iostream>
#include "tracker_lib.h"
using namespace std;

int main(int argc, char *argv[]) {
	Tracker tracker(argv[1]);
	if (!tracker.createServer()) {
		cerr << "Tascker server create error" << endl;
		return 0;
	}
	return 0;
}