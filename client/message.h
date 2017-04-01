#ifndef __MSG_H__
#define __MSG_H__
#include <project.h>
enum Message_t {
	HANDSHAKE,
	HANDSHAKE_REPLY,
	BITFIELD,
	REQUEST,
	REPLY,
	HAVE
};
class Message
{
private:
	int length_pre;
	Message_t message_id;
	string payload;

public:
	Message();
	~Message();
	Messagc(int length, Message_t, payload);
	void set_Message(int length, Message_t mes_type, string payload);
	string get_Message();
};
#endif