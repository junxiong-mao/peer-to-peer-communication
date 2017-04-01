#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include "client.h"
#include <stdexcept>
#include "../util.h"
#include "../socket.h"

Client::Client(char *config) {
    ifstream iConfig(config);
    iConfig >> server_addr.ip >> server_addr.port;
    iConfig >> tracker_addr.ip >> tracker_addr.port;
    iConfig >> file_dir;
}

Client::~Client() {};

bool Client::connect_Tracker(int &sock){
    cout << atoi(tracker_addr.port.c_str())<<endl;
    cout << tracker_addr.ip.c_str()<<endl;
    sock = init_socket(tracker_addr.ip.c_str(), atoi(tracker_addr.port.c_str()));
    return (sock < 0) ? false : true;
}

bool Client::connect_Server(int &sock){
    cout<<atoi(server_addr.port.c_str())<<endl;
    cout<<server_addr.ip.c_str()<<endl;
    sock = init_socket(server_addr.ip.c_str(), atoi(server_addr.port.c_str()) );
    return (sock < 0) ? false : true;
}

bool Client::publish_torrent(const string& filename) {
    Torrent* torrent = new Torrent();
    if (!(torrent->torrent_creation(file_dir, filename))) {
        return false;
    }
    printf("%s torrent created!\n", filename.c_str());
    if (!(torrent -> publish_torrent(this -> server_addr))) {
        return false;
    }
    printf("%s torrent published!\n", filename.c_str());
    if (!registeration(torrent->get_Hash_Code(), torrent -> get_total_piece(), torrent -> get_tor_name())) {
        return false;
    }
    printf("%s torrent registered!\n", filename.c_str());
    delete torrent;
    return true;
}

bool Client::registeration(const string& hashcode, int piece_num, const string torName){
    int sock;
    if (!connect_Tracker(sock)) return false;

    string message = "register";
    send_data(sock, message.c_str(), message.size());
    char *mesg_rec;
    receive_msg(sock, &mesg_rec);
    if (strcmp(mesg_rec, "confirm") != 0) return false;
    string bit_field(piece_num, '1');
    message = hashcode + " " + bit_field + " " + torName; 
    send_data(sock, message.c_str(), message.size() );

    delete[] mesg_rec;
    return true;
}

bool Client::get_Tor_List_File(){
    int sock;
    if(!connect_Server(sock)) return false;

    string message = "torrent_list";
    send_data(sock, message.c_str(), message.size());
    char *mesg_rec;
    receive_msg(sock, &mesg_rec);
    
    if(strcmp(mesg_rec, "confirm") != 0) {
        delete[] mesg_rec;
        return false;
    }
    FILE* fp;
    fp = fopen(torrent_list_name.c_str() ,"wb");
    receive_file(sock, fp);
    fclose(fp);
    delete[] mesg_rec;
    return true;
}

void Client::show_tor_list() {
    ifstream tor_list(torrent_list_name.c_str());
    string whole;
    while(getline(tor_list, whole)) {
        cout << whole << endl;
    }
}

string Client::chosen_Torrent(const string torrent_name) {
    ifstream tor_list(torrent_list_name.c_str());
    string whole;
    string name;
    string hash;
    while(getline(tor_list, whole))
    {
        istringstream istream(whole);
        cout << whole << endl;
        istream >> hash >> name;
        if (name == torrent_name) {
            return hash;
        }
    }
    cout << "returning" << endl;
    return "";
}

bool Client::download_Torrent(string torrent_chosen_hash, string tor_name){
    int sock;
    if(!connect_Server(sock)) return false;

    string message = "download_torrent";
    send_data(sock, message.c_str(), message.size());
    int length = receive_len(sock);
    char *mesg_rec = new char[length];
    receive_data (sock, mesg_rec, length);
    if(strcmp(mesg_rec, "confirm") != 0) {
        delete[] mesg_rec;
        return false;
    }

    message = torrent_chosen_hash;
    send_data(sock, message.c_str(), message.size());
    FILE* fp;
    // tor_name += ".torrent";
    string torDir = file_dir + "/" + tor_name;
    fp = fopen(torDir.c_str(),"wb");
    if (fp == NULL) {
        perror("Open file failed!");
        return false;
    }
    receive_file(sock, fp);
    fclose(fp);
    // update torrent_list
    cout << "Torrent download finished"<< endl;
    Torrent temp(file_dir, tor_name);
    personal_tor_list.push_back(temp);
    delete[] mesg_rec;
    return true;
}

void *callHandler(void *type) {
    BigPack* bp = static_cast<BigPack*>(type);
    return bp -> f ->file_handler(bp -> port);
}

void *callListenPeerHandler(void *type) {
    ClientPack* sp = static_cast<ClientPack*>(type);
    return (sp->client->listen_peer(sp->sock));
}

void *callListenHandler(void *type) {
    ClientPack* sp = static_cast<ClientPack*>(type);
    return (sp->client->listen_file(sp->sock));
}
void* Client::listen_file(void* arg) {
    int sock = *(int*)arg;
    char *msg;
    receive_msg(sock, &msg);
    if (!strcmp(msg, "handshake")) {
        if (send_data(sock, "confirm", 7) < 0) {
            return NULL;
        }
        handshake_reply(sock);
    } else if (!strcmp(msg, "broadcast")) {
        //cout << "get broadcast" << endl;
        if (send_data(sock, "confirm", 7) < 0) {
            return NULL;
        }
        char *piece_info;
        receive_msg(sock, &piece_info);
        string piece_info_str(piece_info);
        istringstream piece_info_stream(piece_info_str);
        int piece_num;
        string tor_name, client_port, client_ip;
        piece_info_stream >> piece_num >> tor_name >> client_port >> client_ip;

        try {
            File *file = launched_files.at(tor_name);
            Address append;
            append.ip = client_ip;
            append.port = client_port;
            for (unsigned int i = 0; i < file -> piece_status[piece_num] -> target_client.size(); i++) {
                if (file -> piece_status[piece_num] -> target_client[i].ip == append.ip &&
                    file -> piece_status[piece_num] -> target_client[i].port == append.port) {
                    return NULL;
                }
            }
            file -> piece_status[piece_num] -> target_client.push_back(append);
            //cout << "registered " << piece_num << " from " << client_ip << "  " << client_port << endl;
        } catch (out_of_range e) {
            return NULL;
        }

    }
    
    return 0;
}

void* Client::listen_peer(void* arg) {
    int port = *(int*)arg;
    cout << "listen peer @" << port << endl;
    int client_sock, c, *new_sock;
    struct sockaddr_in client;
    int socketDesc = init_socket(NULL, port);
    c = sizeof(sockaddr_in);

    while((client_sock = accept(socketDesc, (sockaddr *)&client, (socklen_t*)&c))) {
        //cout << endl << "Connection accepted" << endl;
        //cout << "The peer client is " << endl;
        //cout << inet_ntoa(client.sin_addr) << endl;
        //cout << ntohs(client.sin_port) << endl;

        pthread_t listenThread;
        new_sock = new int;
        ClientPack *sp = new ClientPack;
        sp -> client = this;
        sp -> sock = new_sock;
        *new_sock = client_sock;
        if (pthread_create(&listenThread, NULL, callListenHandler, static_cast<void *>(sp)) < 0) {
            //cerr << "could not create listen thread" << endl;
        }
        //cout << "Handler assigned" << endl;
    }
    
    if (client_sock < 0) {
        //cerr << "accept failed" << endl;
    }
    return 0;
}

void Client::start_torrent(const string& torrent_name) {
    Torrent *torrent = new Torrent(file_dir, torrent_name);
    File *file = new File(torrent);
    launched_files[torrent_name] = file;
    file->tracker_addr = tracker_addr;
    pthread_t *fileThread = new pthread_t;
    int port = 0;
    
    if (!file -> downloaded()) {
        file -> rqst_peer_list(port, false);
        
    } else {
        file -> rqst_peer_list(port, true);
        cout << "downloaded" << endl;
    }
    ClientPack* client_pack = new ClientPack;
    client_pack->client = this;
    client_pack->sock = new int;
    *(client_pack->sock) = (port == 0) ? LISTEN_PORT : port;
    if (pthread_create(&listenThread, NULL, &callListenPeerHandler, static_cast<void*>(client_pack)) < 0) {
        cerr << "could not create listen peer thread" << endl;
        return;
    }
    launch_file_list[torrent_name] = *fileThread;
    BigPack *bp = new BigPack;
    bp -> f = file;
    bp -> port = port;
    if (!file -> downloaded()) {
        cout << "not downloaded" << endl;
        if (pthread_create(fileThread, NULL, &callHandler, static_cast<void*>(bp)) < 0) {
            cerr << "could not create download thread" << endl;
            return;
        }
    }
    
    cout << "started " << torrent_name << endl;
}

void Client::stop_torrent(const string& torrent_name) {
    Torrent torrent(file_dir, torrent_name);
    map<string ,pthread_t>::iterator l_it;
    l_it = launch_file_list.find(torrent_name);
    if (l_it == launch_file_list.end()) {
        cerr << "No such thread! ERROR!" << endl;
        return;
    }

    launch_file_list.erase(l_it);
}

bool Client::compare_Info_Hash(string torrent_chosen_hash, string torrent_name){
    string hash_cal;
    FILE *torrent;
    torrent = fopen(torrent_name.c_str(), "rb");
    if(torrent == NULL){
        cout << "Fail to open original file."<<endl;
        return 1;
    }
    int size = get_file_size(torrent_name.c_str());
    hash_cal = sha1sum(torrent, size);
    fclose(torrent);
    return (hash_cal == torrent_chosen_hash);
}

bool Client::request_reply(string file_name, const int &sock){
    char *msg_rec;
    receive_msg(sock, &msg_rec);
    if (!strcmp(msg_rec,"end_request")) return false;
    int piece_number = atoi(msg_rec); 
    string piece_num = msg_rec;
    // first get the piece number of wanted file
    FILE *fp;
    fp = fopen(file_name.c_str(), "rb");
    if(fp) {
        int f1, f2, size;
        fseek(fp, 0, SEEK_END);
        f2 = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        fseek(fp, FILE_SIZE*piece_number, SEEK_SET); // the first piece is piece 0
        f1 = ftell(fp);
        size = f2 - f1;

        int send_size;
        if (size < FILE_SIZE){
            cout << "sending last data piece." << endl;
            send_size = send_data(sock, NULL, size, fp); // the last piece
        }
        else {
            cout << "sending " << piece_num << " piece" << endl;
            send_size = send_data(sock, NULL, FILE_SIZE, fp);
        }
        cout << "sended size is " << send_size << endl;
        fclose(fp);
    }
    else {
        string filepiece = file_name + "_" + piece_num;
        cout << filepiece << endl;
        FILE *fp_seg;
        fp_seg = fopen(filepiece.c_str(),"rb");
        if(fp_seg != NULL){
            send_data(sock,NULL, get_file_size(filepiece.c_str()), fp_seg);
            fclose(fp_seg);
        }
    }
    return true;
}

void Client::handshake_reply(const int &sock){
    //cout << "handshake replying" << endl;
    
    char *msg_rec;
    receive_msg(sock, &msg_rec);
    string torrent_name = msg_rec;
    Torrent *tor = new Torrent(file_dir, torrent_name);
    //cout << "handshake received" << endl;

    while(request_reply(tor -> get_file_path() ,sock)) {};
    //cout << "request end" << endl;
}
