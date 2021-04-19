#include "DHT11.h"

static unsigned char pin = 0;

//#define _NOP() do { __asm__ volatile ("nop"); } while (0)
void dht11_init(unsigned char pin1)
{
	pin = pin1;
}


//信号脚拉低 >18ms 这里只负责拉低，延时交给中断处理
char dht11_read_ready()
{
	// Request sample
	pinMode(pin, OUTPUT);
	digitalWrite(pin, LOW); // Send start signal
	return LOW_PIN_ms;
}
char dht11_read_data(struct DHT11_data* value)
{
	pinMode(pin, INPUT);
	digitalWrite(pin, HIGH); // Switch bus to receive data
	
	unsigned short rawHumidity = 0;
	unsigned short rawTemperature = 0;
	unsigned short data = 0;
	/*
	uint16_t rawHumidity = 0;
	uint16_t rawTemperature = 0;
	uint16_t data = 0;
	*/
	unsigned long startTime;
	//   cli();
	//noInterrupts();
	for (short i = -3; i < 2 * 40; i++)
	{
		byte age;
		startTime = micros();

		do
		{
			age = (unsigned long)(micros() - startTime);
			if (age > 90)
			{
			
				// sei();
				//interrupts();
				return 0;
			}
		} while (digitalRead(pin) == (i & 1) ? HIGH : LOW);

		if (i >= 0 && (i & 1))
		{
			// Now we are being fed our 40 bits
			data <<= 1;

			// A zero max 30 usecs, a one at least 68 usecs.
			if (age > 30)
			{
				data |= 1; // we got a one
			}
		}

		switch (i)
		{
		case 31:
			rawHumidity = data;
			break;
		case 63:
			rawTemperature = data;
			data = 0;
			break;
		}
	}
	//   sei();
	//interrupts();
	// Verify checksum

	if ((byte)(((byte)rawHumidity) + (rawHumidity >> 8) + ((byte)rawTemperature) + (rawTemperature >> 8)) != data)
	{
		return  -1;
	}

	value->humidity = (rawHumidity >> 8) + ((rawHumidity & 0x00FF) * 0.1);
	value->temperature = (rawTemperature >> 8) + ((rawTemperature & 0x00FF) * 0.1);
	//value->humidity = rawHumidity >> 8;
	//value->temperature = rawTemperature >> 8;
	return  1;
}
