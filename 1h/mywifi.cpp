#include "mywifi.h"

extern "C"
{
    uint8_t anjian1 = 0; //按键1输入

    void set_anjian1(const uint8_t pin)
    {
        anjian1 = pin;
    }

    //从文加里读取wifi账号和密码，储存在 WIFI_ssid   WIFI_password里
    short file_read_wifidata(char *WIFI_ssid, char *WIFI_password, const char *wifi_ssid_pw_file)
    {
        //Serial.println("SPIFFS format start");
        //LittleFS.format();    // 格式化SPIFFS
        //Serial.println("SPIFFS format finish");
        if (LittleFS.begin())
        { // 启动SPIFFS
            Serial.println("SPIFFS Started.");
        }
        else
        {
            Serial.println("SPIFFS Failed to Start.");
            return -2; //文件系统启动失败
        }
        //确认闪存中是否有file_name文件
        if (LittleFS.exists(wifi_ssid_pw_file))
        {
            Serial.print(wifi_ssid_pw_file);
            Serial.println(" FOUND.");
        }
        else
        {
            Serial.print(wifi_ssid_pw_file);
            Serial.print(" NOT FOUND.");
            return -1; //未找到WiFi文件
        }
        //建立File对象用于从SPIFFS中读取文件
        File dataFile1 = LittleFS.open(wifi_ssid_pw_file, "r");

        /*
	规定  /wifidata.txt 此文件储存WiFi的账号和密码
	第一行储存账号	WIFI_ssid
	第二行储存密码	WIFI_password
	两行都采用\n 作为结束符
	*/

        //读取文件内容并将文件内容WiFi账号和密码写入数组
        int i;
        if (dataFile1.size() > WIFI_SSID_LEN + WIFI_PASSWORD_LEN)
        {
            dataFile1.close(); //推出之前关闭文件，防止未知错误
            return -3;         //文件长度错误
        }
        for (i = 0; i < dataFile1.size() && i < WIFI_SSID_LEN; i++)
        {
            WIFI_ssid[i] = (char)dataFile1.read();
            if (WIFI_ssid[i] == '\n' || WIFI_ssid[i] == '\0')
            {
                WIFI_ssid[i] = '\0';
                break;
            }
        }
        for (; i < WIFI_SSID_LEN; i++)
        {
            WIFI_ssid[i] = '\0';
        }
        for (i = 0; i < dataFile1.size() && i < WIFI_PASSWORD_LEN; i++)
        {
            WIFI_password[i] = (char)dataFile1.read();
            if (WIFI_password[i] == '\n' || WIFI_password[i] == '\0')
            {
                WIFI_password[i] = '\0';
                break;
            }
        }
        for (; i < WIFI_PASSWORD_LEN; i++)
        {
            WIFI_password[i] = '\0';
        }
        //完成文件读取后关闭文件
        dataFile1.close();
        return 1;
    }

    //将  WIFI_ssid  WIFI_password 保存到 wifi_ssid_pw_file 命名的文件里
    short file_save_wifidata(char *WIFI_ssid, char *WIFI_password, const char *wifi_ssid_pw_file)
    {
        if (!LittleFS.begin())
        { // 启动SPIFFS
            Serial.println("SPIFFS Failed to Start.");
            return -2; //文件系统启动失败
        }
        //直接 ‘w’ 的话会被直接清除掉的，似乎不用我删除
        /*
	规定  /wifidata.txt 此文件储存WiFi的账号和密码
	第一行储存账号	WIFI_ssid
	第二行储存密码	WIFI_password
	两行都采用\n 作为结束符
	*/
        File dataFile = LittleFS.open(wifi_ssid_pw_file, "w"); // 建立File对象用于向SPIFFS中的file对象（即/notes.txt）写入信息
        dataFile.print(WIFI_ssid);
        dataFile.print('\n');
        dataFile.print(WIFI_password);
        dataFile.print('\n');
        dataFile.close(); // 完成文件写入后关闭文件
        return 1;
    }

    //删除文件
    short file_delete(const char *file_name)
    {
        if (LittleFS.begin())
        { // 启动SPIFFS
            Serial.println("SPIFFS Started.");
        }
        else
        {
            Serial.println("SPIFFS Failed to Start.");
            return -2; //文件系统启动失败
        }

        //确认闪存中file_name文件是否被清楚
        if (LittleFS.exists(file_name) && LittleFS.remove(file_name))
        {
            Serial.print(file_name);
            Serial.println(" found && delete");
        }
        else
        {
            Serial.print(file_name);
            Serial.print("NOT FOUND");
        }
        return 1;
    }

    /*连接wifi 这个函数会占用大量的时间，用于连接wifi，调用中断函数可能导致连接失败
    WIFI_ssid
    WIFI_password
    anjian1 按键引脚，长按删除文件 wifi_ssid_pw_file 并复位程序
    */
    short get_wifi(char *WIFI_ssid, char *WIFI_password, const char *wifi_ssid_pw_file)
    {
        Serial.print("Connecting to ");
        Serial.println(WIFI_ssid);
        Serial.print("WIFI_password ");
        Serial.println(WIFI_password);
        WiFi.mode(WIFI_STA);
        WiFi.begin(WIFI_ssid, WIFI_password);
        short i = 20;

        pinMode(anjian1, INPUT);      //按键1
        pinMode(LED_BUILTIN, OUTPUT); //小板子的上的led
        while ((WiFi.status() != WL_CONNECTED) && i-- > 0)
        {
            delay(500);
            Serial.print(".");

            //长按 i*500 ms
            //删除之前记住的WiFi账号和密码，然后重新启动系统
            if (digitalRead(anjian1) == LOW)
            {
                //亮灯指示一下
                digitalWrite(LED_BUILTIN, LOW);
                short count_anjian = 0; //对按键按下的时间计数，超过5s就清除wifidata.txt文件，然后重新启动系统
                while (digitalRead(anjian1) == LOW && count_anjian < 10)
                {
                    Serial.printf("-%dS -", 5 - count_anjian/2);
                    count_anjian = count_anjian + 1;
                    delay(500);
                }
                if (count_anjian >= 10)
                {
                    //删除之前记住的WiFi账号和密码，然后重新启动系统
                    Serial.println("delete & restart");
                    file_delete(wifi_ssid_pw_file);
                    ESP.restart();
                }
                digitalWrite(LED_BUILTIN, HIGH);
            }
        }
        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("WiFi connected");
            Serial.println("IP address: ");
            Serial.println(WiFi.localIP());
            return 1;
        }
        else
        {
            return 0;
        }
    }

    /*
    成为一个wifi热点，名字  WIFI_SERVER_NAME ，密码  WIFI_SERVER_PASSWORD，开放tcp端口 SERVER_CLIENT_PROT
    tcp接收需要链接的wifi的名字、密码和用户的绑定uid，仅支持ASCII码
    接收到的wifi账号和密码会储存在 WIFI_ssid WIFI_password 和文件wifi_ssid_pw_file里
    UID通过应用调用的方式返回数据
    函数的返回值仅返回状态，目前不收到足够的数据，绝不返回。while(1);

    */
    short tcp_server_get_wifi_data(char *WIFI_ssid, char *WIFI_password, unsigned long long &UID, uint32_t CHIP_ID, const char *wifi_ssid_pw_file)
    {
        //WiFi.mode()
        char data[1024];
        int ind = 0;
        //WiFi.mode(WIFI_RESUME);//更新到了3.xx的esp8266之后这个宏定义不见了
        IPAddress softLocal(192, 168, 128, 1); // 1 设置内网WIFI IP地址
        IPAddress softGateway(192, 168, 128, 1);
        IPAddress softSubnet(255, 255, 255, 0);
        WiFi.mode(WIFI_AP);
        WiFi.softAPConfig(softLocal, softGateway, softSubnet);
        WiFi.softAP(WIFI_SERVER_NAME, WIFI_SERVER_PASSWORD);
        Serial.print("Connected to ");
        Serial.println(WIFI_SERVER_NAME);
        Serial.print("password = ");
        Serial.println(WIFI_SERVER_PASSWORD);
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP()); //串口监视器显示IP地址

        WiFiServer server(SERVER_CLIENT_PROT); //端口，自定义（避免公用端口）
        WiFiClient client;
        // Start the TCP server
        server.begin();
        while (1)
        {
            if (!client.connected())
            {
                //try to connect to a new client
                client = server.available();
            }
            else
            {
                if (client.available() > 0)
                {
                    //Serial.println("Connected to client");

                    while (client.available())
                    {
                        data[ind] = client.read(); //读取client端发送的字符
                        ind++;
                    }
                    client.flush();
                    //处理其他设备发送过来的数据
                    //找到"wifi:"字符
                    if (str1_find_str2_(data, ind, "+SSID:") >= 0)
                    {
                        for (int i = 6, k = 0; i < ind && i < WIFI_SSID_LEN; i++, k++)
                        {
                            WIFI_ssid[k] = data[i];
                            data[i] = 0; //转移了的数据清零
                            if (WIFI_ssid[k] == '\n' || WIFI_ssid[k] == '\0')
                            {
                                WIFI_ssid[k] = '\0';
                                break;
                            }
                        }
                        client.print(WIFI_ssid); //在client端回复
                    }
                    else if (str1_find_str2_(data, ind, "+PW:") >= 0)
                    {
                        for (int i = 4, k = 0; i < ind && i < WIFI_PASSWORD_LEN; i++, k++)
                        {
                            WIFI_password[k] = data[i];
                            data[i] = 0; //转移了的数据清零
                            if (WIFI_password[k] == '\n' || WIFI_password[k] == '\0')
                            {
                                WIFI_password[k] = '\0';
                                break;
                            }
                        }
                        client.print(WIFI_password); //在client端回复
                    }
                    else if (str1_find_str2_(data, ind, "+UID:") >= 0)
                    {
                        //len("+UID:")=4
                        short status=0;
                        UID = str_to_u64(data + 4, ind - 4,&status);
                        if(status!=1)
                        {
                            client.printf("error UID not found!", UID); //在client端回复
                            Serial.print(data);
                            Serial.print("error UID not found!\r\n");
                            client.stop();
                        }
                        data[0] = 0;                  //转移了的数据清零
                        client.printf("UID:%llu", UID); //在client端回复
                    }
                    else
                    {
                        continue;
                    }
                    for (int j = 0; j < ind; j++)
                    {
                        Serial.print(data[j]);
                    }
                    Serial.print("#WIFI_ssid:");
                    Serial.print(WIFI_ssid);
                    Serial.print("#WIFI_password:");
                    Serial.print(WIFI_password);
                    Serial.print('#');
                    Serial.print("\n");
                    if (WIFI_ssid[0] != 0 && WIFI_password[0] != 0)
                    {
                        delay(5);                                          //延时五毫秒，不然最后一个数据可能发不出去
                        client.printf(",uid=%llu,chip_id=%d", UID, CHIP_ID); //在client端回复
                        delay(5);                                          //延时五毫秒，不然最后一个数据可能发不出去
                        file_save_wifidata(WIFI_ssid, WIFI_password, wifi_ssid_pw_file);
                        return 1;
                    }
                    ind = 0;
                }
            }
        }
    }

    /*检测按键按下，超过 CLEAR_WIFI_DATA_S 后删除之前记住的WiFi账号和密码，然后重新启动系统*/
    void clear_wifi_data(const char *wifi_ssid_pw_file)
    {
        static short count_anjian = 0; //对按键按下的时间计数，超过5s就清除wifidata.txt文件，然后重新启动系统
        //长按 25*TIMER1_timeout_ms ms
        //删除之前记住的WiFi账号和密码，然后重新启动系统
        if (digitalRead(anjian1) == LOW)
        {

            digitalWrite(LED_BUILTIN, LOW);
            count_anjian++;
            Serial.printf("-%dS -", (CLEAR_WIFI_DATA_COUNT - count_anjian) * TIMER1_timeout_ms / 1000);
            if (count_anjian > CLEAR_WIFI_DATA_COUNT)
            {
                //删除之前记住的WiFi账号和密码，然后重新启动系统
                Serial.println("delete & restart");
                file_delete(wifi_ssid_pw_file);
				file_delete(stut_data_file);
                ESP.restart();
            }
        }
        else
        {
            digitalWrite(LED_BUILTIN, HIGH);
            count_anjian = 0;
        }
    }
}