#include "test.h"
extern "C"
{
void get_files_name()
{
	Dir dir;
	dir = LittleFS.openDir("/");  // 建立“目录”对象
	while (dir.next())
	{  // dir.next()用于检查目录中是否还有“下一个文件”
		Serial.println(dir.fileName()); // 输出文件名
	}
	Serial.println("end"); // 输出文件名
}


}
