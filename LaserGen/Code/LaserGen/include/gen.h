/**
 * @file gen.h
 * @author Patryk Sienkiewicz (@Patsen95)
 * @version 0.1
 * 
 * 
 *
 */

#pragma once

#include "dacxx6x.h"
#include "util.h"
#include "cmdparser.h"


#define TIMER_DIVIDER 	80

#define SIG_PEAK		16384
#define MAX_AMPLITUDE 	(SIG_PEAK / 2)

#define MAX_PHASE_CNT		1639

// This param influences resolution of the generated signal
#define SAMPLES_PER_SECOND	1000

#define MICROS_PER_SECOND	1000000UL	// ESP32 timer max resolution
#define MICROS_PER_SAMPLE	(MICROS_PER_SECOND / SAMPLES_PER_SECOND)


typedef enum
{
	ARBITRARY = 0,
	SINE,
	SAW
} wavetype_t;

typedef struct
{
	float frequency;
	float amplitude;
	float phase;
	float offset;
	uint16 *wavetable;
	wavetype_t waveType;
} osc_t;


/*
	1. Generate base sine wavetable
	2. Modify it in output buffer
	3. Send it to dac on timer event running with precalculated frequency
 */

class WaveGen
{
public:
	WaveGen();
	~WaveGen();

	void init();


	void enable();
	void disable();

	// void sweep(uint16 start, uint16 end);

	osc_t m_sineOsc;
private:
	hw_timer_t *m_timerSine;
	hw_timer_t *m_timerSaw;
	dac8162 *m_dac;

	uint16 *m_phaseBuf_sin;
	uint16 *m_phaseBuf_saw;

	osc_t m_sawOsc;

	uint16 interpolate(uint16 *w_tab, uint16 index);

	void onTimer_Sin();
	void onTimer_Saw();
};


