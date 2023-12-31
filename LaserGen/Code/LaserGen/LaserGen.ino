#include "gen.h"

dac8162 dac;


void setup()
{
	//Serial.begin(115200);

	dac.attachClear(2);
	dac.init();

	dac.ch_a->enable();
	dac.ch_a->setVoltage(0.53f);
}

void loop()
{

}
