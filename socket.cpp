#include "socket.h"

int receive_len(int sock) {
    int recv_size = 0;
    char* buf = new char[1];
    string msg_len;
    while (true) {
        int cur_size = recv(sock, buf, 1, 0);
        if (cur_size < 0) {
            puts("recv length failed!");
            delete[] buf;
            return -1;
        }
        if (*buf == '\0') break;
        msg_len += *buf;
        recv_size += cur_size;
    }
    delete[] buf;
    return atoi(msg_len.c_str());
}

int receive_data(int sock, char* rec, int size, FILE* file) {
    int recv_size = 0;
    int cur_size = 0;
    int write_size = 0;

    while (recv_size < size) {
        if (file != NULL) {
            cur_size = recv(sock, rec, 10*BUF_SIZE, 0);
        } else {
            cur_size = recv(sock, rec + recv_size, size - recv_size, 0);
        }
            
        if (cur_size <= 0) {
            puts("recv failed!");
            return -1;
        }

        recv_size += cur_size;
        // printf("%d\r", recv_size);
        if (file != NULL) {
            if (recv_size > size)
                cur_size = cur_size - (recv_size - size);
            write_size = fwrite(rec, sizeof(char), cur_size, file);
            if (write_size < cur_size) {
                puts("write failed!");
                return -2;
            }
            memset(rec, 0, 10*BUF_SIZE);
        }   
    }
    return recv_size;
}

int receive_file(int sock, FILE* file) {
    int len = receive_len(sock);
    if (len < 0) return -1;
    char* buf = new char[10*BUF_SIZE];
    int status = receive_data(sock, buf, len, file);
    delete[] buf;
    return status;
}

int receive_msg(int sock, char** recv) {
    int len = receive_len(sock);
    if (len < 0) return -1;
    *recv = new char[len+1];
    int status = receive_data(sock, *recv, len);
    (*recv)[len] = 0;
    return status;
}

int send_data(int sock, const char* data, int size, FILE* file) {
    int send_size = 0;

    ostringstream oss;
    oss << size;
    string header = oss.str();

    // send header
    int status = send(sock, header.c_str(), header.size()+1, MSG_NOSIGNAL);
    if (status < 0) {
        perror("send header failed. Error");
        return -1;
    }
    // send data
    if (file != NULL) {
        char* buf = new char[BUF_SIZE];
        int read_size;
        while( (read_size = fread(buf, 1, BUF_SIZE, file))>0 && send_size < size) {
            int ready_size = (read_size + send_size > size)?
                            size - send_size : read_size;
            status = send(sock, buf, ready_size, MSG_NOSIGNAL);
            if (status < 0) {
                perror("send data failed. Error");
                delete [] buf;
                return -1;
            }
            send_size += ready_size;
            memset(buf,0, BUF_SIZE);
        }
        delete [] buf;
    } else {
        status = send(sock, data, size, MSG_NOSIGNAL);
        if (status < 0) {
            perror("send data failed. Error");
            return -1;
        }
    }
    return send_size;
}

int init_socket(const char* addr, int port, bool quiet) {
    struct sockaddr_in server;
    bzero(&server, sizeof(server));

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Could not create socket");
        return -1;
    }

    if (addr == NULL)
        server.sin_addr.s_addr = INADDR_ANY;
    else
        server.sin_addr.s_addr = inet_addr(addr);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    if (addr != NULL) {
        if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0) {
            if (!quiet)
                perror("connect failed. Error");
            return -1;
        }
        if (!quiet)
            puts("Connected\n");
        return sock;
    }
 
    int yes = 1;
    int status = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if (status < 0){
        perror("set options failed. Error");
        return -1;
    }

    status = bind(sock,(struct sockaddr *)&server , sizeof(server));
    if(status < 0) {
        perror("bind failed. Error");
        return -1;
    }

    int length = sizeof(server);
    status = getsockname(sock, (struct sockaddr*) &server, (socklen_t*) &length);
    if (status < 0){
        perror("get socket name failed. Error");
        return -1;
    }

    status = listen(sock, MAX_CONS);
    if(status < 0) {
        perror("listen failed. Error");
        return -1;
    }
    puts("Waiting for incoming connections ...");

    cout << "@port " << ntohs(server.sin_port) << endl;

    return sock;
}

