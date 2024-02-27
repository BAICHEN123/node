## Repository for "esp8266" End Code of an IoT Project

Thanks Chat-GPT for the translation.


### New Content
1. Support for OTA (Over-the-Air updates) has been added.  
After compiling, copy the generated bin file to the corresponding location under MyConfig.OTA_DIR_NAME directory. The files will be classified based on the compilation date and character information contained in the bin file. Update decisions for corresponding nodes will be made based on the OTA_SERVER_FIND_TAG information sent by the nodes.  
Firmware of approximately 344400 bytes is transmitted and written in approximately 6109ms, much faster than burning programs via serial port.  
~~After I finish implementing the node logging feature, I will attempt to debug the program using OTA. After all, breakpoints debugging is not feasible, and OTA burning is faster.~~

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