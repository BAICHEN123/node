#ifndef _TEST_h
#define _TEST_h

#include <LittleFS.h>

#ifdef __cplusplus
extern "C" {
#endif

    //不知道为什么，我无法在这里进行 const 变量的全局定义，但是我会在之后尝试在一个指定的文件里定义我所有的全局变量，并注释
    void get_files_name();

    
#ifdef __cplusplus
}
#endif

#endif
