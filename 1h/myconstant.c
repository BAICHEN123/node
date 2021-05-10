#include "myconstant.h"

void resetFunc()
{
    void (*resetFunc1)(void) = 0;
    resetFunc1();
}
