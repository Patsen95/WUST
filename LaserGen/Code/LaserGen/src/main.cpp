#include <Arduino.h>

#include "gen.h"


WaveGen wg;
CmdParser parser;


void setup()
{
	Serial.begin(115200);

	// dac.init();

	// dac.ch_a->enable();
	// dac.ch_a->setVoltage(0.53f);

	

	wg.init();


}

uint16 i = 0;

void loop()
{
	// if(i < MAX_PHASE_CNT - 1)
	// 	i++;
	// else
	// 	i = 0;

	// Serial.print(">Phase cnt:");
	// Serial.println(i);

	// Serial.print(">Wavetable:");
	// Serial.println(wg.m_sineOsc.wavetable[i]);
	// delay(10);
}

void serialEvent()
{
	static char buf[IN_BUF_SIZE];
	static uint8 chars = 0;

	char ch = Serial.read();

	if(chars < IN_BUF_SIZE - 1)
	{		
		buf[chars] = ch;
		chars++;

		if(ch == '\n')
		{
			parser.parse(buf, chars);
			chars = 0;
			memset(buf, 0, (size_t)IN_BUF_SIZE);
		}
	}
}
