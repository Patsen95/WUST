#include <Arduino.h>

#include "gen.h"


WaveGen wg;

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
	if(i < MAX_PHASE_CNT - 1)
		i++;
	else
		i = 0;

	Serial.print(">Phase cnt:");
	Serial.println(i);

	Serial.print(">Wavetable:");
	Serial.println(wg.m_sineOsc.wavetable[i]);
	delay(10);
}
