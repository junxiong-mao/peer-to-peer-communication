#include "util.h"

int get_file_size(const char* filename) {
    struct stat file_stat;
    int status = stat(filename, &file_stat);
    if (status < 0) perror("get file size failed");
    return (status < 0) ? -1 : file_stat.st_size;
}

string sha1sum(FILE* file, const int file_size) {
	SHA_CTX s;
	int i, size;
	unsigned char hash[20];
	char c[512];
	int read_size = 0;
	int buf_size = (file_size - read_size >= 512) ?
					 512 : file_size - read_size;
	if (file_size == 0) buf_size = 512;

	SHA1_Init(&s);
	while ( (size = fread(c, 1, buf_size, file)) > 0) {
  		SHA1_Update(&s, c, size);
  		read_size += size;
  		if (file_size - read_size < 512 && file_size != 0)
  			buf_size = file_size - read_size;
	}
  	SHA1_Final(hash, &s);

	string sha1;
	char buf[2];
	for (i=0; i < 20; i++) {
		sprintf(buf, "%02x", hash[i]);
		sha1 = sha1 + buf[0] + buf[1];
	}

	return sha1;
}
