## Repository for "esp8266" End Code of an IoT Project

Thanks Chat-GPT for the translation.
### Other Ends
#### Gitee
[app](https://gitee.com/he_chen_chuan/Mytabs)  
[BC-server](https://gitee.com/he_chen_chuan/BC-server)  

#### GitHub
[app](https://github.com/BAICHEN123/Mytabs)  
[BC-server](https://github.com/BAICHEN123/BC-server)  

### Description
Developed using Arduino for esp8266.  
#### Currently implemented features:
- Collecting and storing sensor data
- Remote control of LED lights
- Email notifications
- Multiple device integration

#### Contents in this repository:
- Examples: "[new](./new/)", "[new2](./new2/)", "[new_temperature](./new_temperature/)"
- Old code and some data: "[old data](./old%20data/)"
- Common code for multiple examples: "[src](./src/)"

#### How to compile using Arduino IDE?
1. Search in your browser for "how to configure esp8266 environment in Arduino" and complete the setup.
2. Copy or link the "[src](./src/)" directory to the directories of the examples: "[new](./new/)", "[new2](./new2/)", "[new_temperature](./new_temperature/)". Otherwise, header file not found errors may occur.
3. Open one or more of the examples: "[new](./new/)", "[new2](./new2/)", "[new_temperature](./new_temperature/)" in Arduino IDE.
4. Select the esp8266 development board and the serial port connected to the esp8266 development board and your computer in Arduino IDE.
5. Click on the compile or burn button.

#### Configuration

Most constants are placed in [myconstant.h](./src/myconstant.h) and [myconstant.cpp](./src/myconstant.cpp).
Some constants are also placed in their respective files, such as the TCP port for server communication ([mytcp.h](./src/mytcp.h)) and the UDP port ([myudp.h](./src/myudp.h)).