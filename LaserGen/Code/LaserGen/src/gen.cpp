#include "gen.h"

#include <math.h>



// void IRAM_ATTR WaveGen::onTimer_Sin()
// {

// }

// void IRAM_ATTR WaveGen::onTimer_Saw()
// {

// }

/**************************************************************************/
WaveGen::WaveGen() { }
WaveGen::~WaveGen()
{
	delete m_timerSine;
	// delete m_timerSaw;
	delete m_dac;
	delete m_phaseBuf_sin;
	delete m_phaseBuf_saw;
}

void WaveGen::init()
{
	m_sineOsc = {
		.frequency = 100.0f, // 100 Hz
		.amplitude = 1.0f,
		.phase = 0,
		.offset = 0,
		.wavetable = new uint16(MAX_PHASE_CNT),
		.waveType = wavetype_t::SINE
		};

	for (uint16 i = 0; i < MAX_PHASE_CNT - 1; i++)
		m_sineOsc.wavetable[i] = MAX_AMPLITUDE + m_sineOsc.amplitude * MAX_AMPLITUDE * sinf(2.0f * M_PI * i / MAX_PHASE_CNT);

	// m_phaseBuf_sin = new uint16(MAX_PHASE_CNT);



	// Init timers
	// m_timerSine = timerBegin(0, TIMER_DIVIDER, true);
	// m_sawTimer = timerBegin(1, TIMER_DIVIDER, true);

	// m_dac = new dac8162();
	// m_dac->init();

	// timerAttachInterrupt(m_sinTimer, &onTimer_Sin, true);
	// timerAlarmWrite(m_sinTimer, MICROS_PER_SAMPLE, true);
}

// void WaveGen::enableSin()
// {
// 	timerAlarmEnable(m_sinTimer);
// }

// void WaveGen::disableSin()
// {
// 	timerAlarmDisable(m_sinTimer);
// }

// void WaveGen::enableSaw()
// {
// 	timerAlarmEnable(m_sawTimer);
// }

// void WaveGen::disableSaw()
// {
// 	timerAlarmDisable(m_sawTimer);
// }

/**************************************************************************/
uint16 WaveGen::interpolate(uint16 *w_tab, uint16 index)
{

	return 0;
}
