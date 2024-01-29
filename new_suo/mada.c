#include "mada.h"
#define MIN(a, b) ((a) < (b) ? (a) : (b))
/*

type ±1023
正数顺时针，负数逆时针
*/
void __mada_start(struct MyMada *mada, int type)
{

    mada->value = type;
    pinMode(mada->in1, OUTPUT); // 按键1
    pinMode(mada->in2, OUTPUT); // 按键1
    if (type < 0)
    {
        type = -type;
        digitalWrite(mada->in1, LOW);
        analogWrite(mada->in2, type);
        mada->type = TYPE_MADA_FO;
    }
    else
    {
        analogWrite(mada->in1, type);
        digitalWrite(mada->in2, LOW);
        mada->type = TYPE_MADA_O;
    }
    mada->last_set_type_time = millis();
}

void mada_stop(struct MyMada *mada)
{
    pinMode(mada->in1, OUTPUT); // 按键1
    pinMode(mada->in2, OUTPUT); // 按键1
    digitalWrite(mada->in1, HIGH);
    digitalWrite(mada->in2, HIGH);
    mada->type = TYPE_MADA_STOP;
    mada->last_set_type_time = millis();
    mada->value = 0;
}
void mada_wait(struct MyMada *mada)
{
    pinMode(mada->in1, OUTPUT); // 按键1
    pinMode(mada->in2, OUTPUT); // 按键1
    digitalWrite(mada->in1, LOW);
    digitalWrite(mada->in2, LOW);
    mada->type = TYPE_MADA_WAIT;
    mada->last_set_type_time = millis();
    mada->value = 0;
}
void mada_start(struct MyMada *mada, int type)
{

    // 正转反转切换的时候需要先刹车一定时间，如果你忘记了，我就阻塞替你刹车

    if ((mada->type == TYPE_MADA_O && type < 0) || (mada->type == TYPE_MADA_FO && type > 0))
    {
        mada_stop(mada);
        delay(150);
    }
    else if ((mada->type == TYPE_MADA_WAIT || mada->type == TYPE_MADA_STOP) && millis() - mada->last_set_type_time < 150)
    {
        mada_stop(mada);
        delay(MIN(150, abs(150 - (millis() - mada->last_set_type_time))));
    }
    __mada_start(mada, type);
}