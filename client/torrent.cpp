#include "torrent.h"
#include "../socket.h"
#include "../util.h"

Torrent::Torrent() {};
Torrent::Torrent(const string file_dir, const string tor_name) {
    tor_Name = tor_name;
    tor_Path = file_dir + "/" + tor_name;
    ifstream torrent_file;
    torrent_file.open(tor_Path.c_str());
    torrent_file >> file_Name;
    torrent_file >> file_size;
    torrent_file >> total_Piece;
    file_Path = file_dir + "/" + file_Name;
    for (int i = 0; i < total_Piece; i++) {
        string hash;
        torrent_file >> hash;
        piece_hash.push_back(hash);
    }
    torrent_file.close();
    FILE *file;
    file = fopen(tor_Path.c_str(), "rb");
    //cout << "torrent path: " << tor_Path << endl;
    hashcode = sha1sum(file, get_file_size(tor_Path.c_str()));
    fclose(file);
};

Torrent::~Torrent() {};

bool Torrent::connect_server(Address server, int& sock){
    sock = init_socket(server.ip.c_str(), atoi(server.port.c_str()));
    return (sock < 0) ? false : true;
}

bool Torrent::torrent_creation(const string fileDir, const string fileName){
    file_Name = fileName;
    tor_Name = fileName + ".torrent";
    file_Path = fileDir + "/" + file_Name;
    tor_Path = fileDir + "/" + tor_Name;
    cout << file_Path << endl;
    
    FILE *ori_file_c;
    ori_file_c = fopen(file_Path.c_str(), "rb");
    if(ori_file_c == NULL){
        cout << "Fail to open original file."<<endl;
        return false;
    }

    ofstream new_torrent(tor_Path.c_str(), ios::binary); 
    if (!new_torrent.is_open()){
        cout << "Fail to create torrent_file." <<endl;
        return false;
    }
    
    int size = get_file_size(file_Path.c_str());
    int remaind = (size % FILE_SIZE > 0)? 1 : 0;
    int num_of_piece = size / (FILE_SIZE) + remaind;
    /* partition the data file*/
    new_torrent << file_Name << "\n";
    char buf[10];
    sprintf(buf, "%d", size);
    string temp = buf;
    new_torrent << temp << "\n";
    sprintf(buf, "%d", num_of_piece);
    temp = buf;
    new_torrent << temp << "\n";
    cout <<"number of piece is: " <<num_of_piece <<endl;

    for (int i = 0; i < num_of_piece; i++) {
        new_torrent << sha1sum(ori_file_c, FILE_SIZE) << endl;
    }
    new_torrent.close();
    fclose(ori_file_c);

    total_Piece = num_of_piece;
    tor_Path = tor_Path;

    //get the hash code for the whole torrent file.
    FILE *torrent;
    cout << "torpath: " << tor_Path << endl;
    torrent = fopen(tor_Path.c_str(), "rb");
    if(torrent == NULL){
        cout << "Fail to open original file."<<endl;
        return false;
    }

    hashcode = sha1sum(torrent);
    fclose(torrent);    
    return true;
}

bool Torrent::publish_torrent(Address server){
    int sock; 
    if (connect_server(server, sock))
    {
        FILE *tor_file;
        tor_file = fopen(tor_Path.c_str(), "rb");
        if(tor_file == NULL){
            cout << "Fail to open original file." << endl;
            return false;
        }

        string message = "publish";
        send_data(sock, message.c_str(), message.size());
        char *mesg_rec;
        receive_msg(sock, &mesg_rec);
        cout << mesg_rec << endl;
        if (!strcmp(mesg_rec,"confirm"))
        {
            cout << "hash is " << hashcode << endl;
            cout << "torrent name is  "<<tor_Name<<endl;
            send_data(sock, hashcode.c_str(), hashcode.size() );
            send_data(sock, tor_Name.c_str(), tor_Name.size() );
            
            int size = get_file_size(tor_Path.c_str());
            cout << " file size is " << size << endl;
            send_data(sock, NULL, size, tor_file);
            cout << "torrent sent."<<endl;

            receive_msg(sock, &mesg_rec);
            cout << "hashcode: " << mesg_rec << endl;
        } else {
            cout << "not confirm" << endl;
        }
        delete[] mesg_rec;
        return true;
    }
    return false;
}

string Torrent::get_Hash_Code(){
    return hashcode;
}

string Torrent::get_file_name(){
    return file_Name;
}

string Torrent::get_tor_name(){
    return tor_Name;
}

string Torrent::get_file_path() {
    return file_Path;
}

int Torrent::get_total_piece(){
    return total_Piece;
}

int Torrent::get_size() {
    return file_size;
}

string Torrent::resolve_tor(int piece_number){
    ifstream torrent(tor_Path.c_str());
    if(!torrent.is_open())
    {
        cout << "torrent can not open." <<endl;
        return "";
    }
    string temp_string;
    for (int i = 0; i < piece_number + 3; i++)
    {
        getline(torrent, temp_string);
    }
    getline(torrent, temp_string);
    torrent.close();

    return temp_string;
}