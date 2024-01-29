#ifndef _MADA_h
#define _MADA_h
#include <arduino.h>


	enum type_mada
	{
		TYPE_MADA_WAIT = 0,
		TYPE_MADA_STOP ,
		TYPE_MADA_O,//顺时针
		TYPE_MADA_FO,//逆时针


	};

struct MyMada
{
    uint8_t in1;
    uint8_t in2;
    enum type_mada type;
    int value;
    unsigned long last_set_type_time;

};

void mada_start(struct MyMada *mada, int type);
void mada_wait(struct MyMada *mada);
void mada_stop(struct MyMada *mada);



#endif


