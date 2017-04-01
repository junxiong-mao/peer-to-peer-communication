#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <fstream>
#include <iostream>
#include <errno.h>
#include "client.h"

using namespace std;

void list(Client &client) {
	client.get_Tor_List_File();
	client.show_tor_list();
};

void get(Client &client, string filename) {
	// cout << filename << endl;
	string torr_hash = client.chosen_Torrent(filename);
	if (torr_hash.empty())
		cout << "ERROR! No such file." << endl;
	else
		client.download_Torrent(torr_hash, filename);
};

void publish(Client &client, string filename) {
	client.publish_torrent(filename);
};

void start(Client &client, string filename) {
	client.start_torrent(filename);
};

void stop(Client &client, string filename) {
	// cout << filename << endl;
	client.stop_torrent(filename);
};

void help() {
	printf("help               : display help infomaion\n");
	printf("list               : get torrent list\n");
	printf("get <filename>     : download torrent file\n");
	printf("publish <filename> : publish torrent\n");
	printf("start <filename>   : start launch file\n");
	printf("stop <filename>    : stop launch file\n");
	printf("quit               : quit client\n");

	return;
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("Usage : %s < client.config\n", argv[0]);
        return -1;
	}
	Client client(argv[1]);
	//client.createServer();
	string command;
	while (true) {
		cout << "<client> ";
		getline(cin, command);
		if (command.compare("list") == 0) {
			list(client);
		}
		else if (command.compare("help") == 0) {
			help();
		}
		else if (command.compare(0,4,"get ") == 0) {
			get(client, command.substr(4));
		}
		else if (command.compare(0,8,"publish ") == 0) {
			publish(client, command.substr(8));
		}
		else if (command.compare(0,5,"stop ") == 0) {
			stop(client, command.substr(5));
		}
		else if (command.compare(0,6,"start ") == 0) {
			start(client, command.substr(6));
		}
		else if (command.compare(0,4,"quit") == 0) {
			cout << "Bye!" << endl;
			break;
		} else {
			printf("client: '%s' is not a client command. See 'help'.\n", command.c_str());
		}
	}

	return 0;
}