#include "myds18b20.h"
#include <OneWire.h>
// static unsigned char pinX = 12;
struct myds18b20 init_ds18b20(unsigned char pin1)
{
    myds18b20 data;
    data.pin = pin1;
    OneWire *ds = new OneWire(pin1); // on pin 10 (a 4.7K resistor is necessary)
    data.obj = ds;
    byte i;
    byte present = 0;
    byte type_s;
    float celsius, fahrenheit;

    if (!ds->search(data.addr))
    {
        Serial.println("No more addresses.");
        Serial.println();
        ds->reset_search();
        delay(250);
        free_myds18b20(&data);
        return data;
    }

    Serial.print("ROM =");
    for (i = 0; i < 8; i++)
    {
        Serial.write(' ');
        Serial.print(data.addr[i], HEX);
    }

    if (OneWire::crc8(data.addr, 7) != data.addr[7])
    {
        Serial.println("CRC is not valid!");
        free_myds18b20(&data);
        return data;
    }
    Serial.println();

    // the first ROM byte indicates which chip
    switch (data.addr[0])
    {
    case 0x10:
        Serial.println("  Chip = DS18S20"); // or old DS1820
        type_s = 1;
        break;
    case 0x28:
        Serial.println("  Chip = DS18B20");
        type_s = 0;
        break;
    case 0x22:
        Serial.println("  Chip = DS1822");
        type_s = 0;
        break;
    default:
        Serial.println("Device is not a DS18x20 family device.");
        free_myds18b20(&data);
        return data;
    }

    ds->reset();
    ds->select(data.addr);
    ds->write(0x44, 1); // start conversion, with parasite power on at the end

    return data;
}

float read_temperature(struct myds18b20 dsdata)
{
    // OneWire ds(dsdata.pin); // on pin 10 (a 4.7K resistor is necessary)
    OneWire *ds = (OneWire *)(dsdata.obj);
    byte i;
    byte present = 0;
    byte type_s;
    static byte data[9];
    float celsius, fahrenheit;
    present = ds->reset();
    ds->select(dsdata.addr);
    ds->write(0xBE); // Read Scratchpad

    // Serial.print(" ");
    for (i = 0; i < 9; i++)
    { // we need 9 bytes
        data[i] = ds->read();
        // Serial.print(data[i], HEX);
        // Serial.print(" ");
    }

    // Convert the data to actual temperature
    // because the result is a 16 bit signed integer, it should
    // be stored to an "int16_t" type, which is always 16 bits
    // even when compiled on a 32 bit processor.
    int16_t raw = (data[1] << 8) | data[0];
    if (type_s)
    {
        raw = raw << 3; // 9 bit resolution default
        if (data[7] == 0x10)
        {
            // "count remain" gives full 12 bit resolution
            raw = (raw & 0xFFF0) + 12 - data[6];
        }
    }
    else
    {
        byte cfg = (data[4] & 0x60);
        // at lower res, the low bits are undefined, so let's zero them
        if (cfg == 0x00)
            raw = raw & ~7; // 9 bit resolution, 93.75 ms
        else if (cfg == 0x20)
            raw = raw & ~3; // 10 bit res, 187.5 ms
        else if (cfg == 0x40)
            raw = raw & ~1; // 11 bit res, 375 ms
                            //// default is 12 bit resolution, 750 ms conversion time
    }
    celsius = (float)raw / 16.0;
    return celsius;
}

void free_myds18b20(myds18b20 *dsdata)
{
    if (dsdata->obj)
    {
        delete (OneWire *)(dsdata->obj);
        dsdata->obj = nullptr;
    }
}