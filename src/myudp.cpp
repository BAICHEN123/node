#include "myudp.h"

/*
UDP发送函数封装起来，方便调用
调用示例 ：UDP_Send(MYHOST, UDP_PORT, "UDP send 汉字测试 !");
return :
            -1	无法链接
            0	发送失败
            1	发送成功
*/
short UDP_Send(const char *UDP_IP, uint16_t UDP_port, String udp_send_data)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        return -1;
    }
    WiFiUDP Udp;
    Udp.beginPacket(UDP_IP, UDP_port);
    Udp.write(udp_send_data.c_str());
    return Udp.endPacket();
}