## 一个物联网项目的"esp8266"端代码仓库

[Thanks Chat-GPT for the translation.](./README.en.md)
### 其他端
#### gitee
[app](https://gitee.com/he_chen_chuan/Mytabs)  
[BC-server](https://gitee.com/he_chen_chuan/BC-server)  

#### github
[app](https://github.com/BAICHEN123/Mytabs)  
[BC-server](https://github.com/BAICHEN123/BC-server)  

目前只差app端代码了，文档慢慢完善吧。

### 简述
使用arduino方式开发esp8266.  
#### 目前实现的功能：
>连接app发送来的wifi  
>采集、储存传感器数据  
>远程点亮LED灯。  
>邮件提示。  
>多设备联动。  

#### 该仓库目前包含的内容：   
>_使用示例  “[new](./new/)”、“[new2](./new2/)”、“[new_temperature](./new_temperature/)”._  
>_旧的代码和一些数据  “[old data](./old%20data/)”_  
>_多个示例的公用代码“[src](./src/)”._  

#### 如何使用arduino编译器编译？
1. 自行在浏览器搜索“arduino如何配置esp8266环境”，并完成配置
2. 将“[src](./src/)”目录复制(copy,cp)或链接(mklink,ln)到示例  “[new](./new/)”、“[new2](./new2/)”、“[new_temperature](./new_temperature/)”目录下。不然会报错找不到头文件。
3. 使用arduino ide打开示例  “[new](./new/)”、“[new2](./new2/)”、“[new_temperature](./new_temperature/)”其中的一个或多个。
4. 在arduino ide中选择esp8266开发板和esp8266开发板与电脑相连的串口号。
5. 点击编译或烧录按键。

#### 配置

大部分常量被放在[myconstant.h](./src/myconstant.h)、[myconstant.cpp](./src/myconstant.cpp)里，
也有一部分在各自的文件里，比如和服务器通讯的tcp端口（[mytcp.h](./src/mytcp.h)）和udp端口（[myudp.h](./src/myudp.h)）

#### 示例内容
具体查看代码里.ino文件中的“struct MyType data_list[MAX_NAME] = {”定义  
[new](./new/)  
dht11采集温湿度，一个mq烟雾传感器在ad转换引脚上，另外接了2个继电器。  
其他还有一些日期、时间、逻辑与门和倒计时  

[new2](./new2/)  
是new的删减版，没有传感器、继电器等外设，仅保留了对ad引脚的采集。

[new_temperature](./new_temperature/)  
采集ds18b20温度传感器等。