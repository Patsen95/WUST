/**
*	@file dacxx6x.cpp
*
*	@author Patryk Sienkiewicz (Patsen95), 2023
*
*	****************************************
*	The MIT License (MIT)
*
*	Permission is hereby granted, free of charge, to any person obtaining a copy
*	of this software and associated documentation files (the "Software"), to deal
*	in the Software without restriction, including without limitation the rights
*	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*	copies of the Software, and to permit persons to whom the Software is
*	furnished to do so, subject to the following conditions:
*
*	The above copyright notice and this permission notice shall be included in
*	all copies or substantial portions of the Software.
*
*	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*	SOFTWARE.
*/

#include "dacxx6x.h"

#include <math.h>


void packData(DataFrame *dt, uint16 data)
{
	// Reducing 16-bit to left-aligned value with width indicated by bitOffset.
	// Since data argument is an unsigned, we can just left-shift it.
	data = data << dt->bitOffset;
	dt->raw[1] = (data & 0xff00) >> 8;	// higher byte
	dt->raw[2] = data & 0x00ff;			// lower byte
}

void packAddress(DataFrame *dt, uint8 addr)
{
	if(addr > 0x7)
		addr = 0x7;
	uint8 a = dt->tail;
	dt->raw[0] = (a & ~ADDRESS_MASK) | (addr & ADDRESS_MASK);
}

void packCmd(DataFrame *dt, uint8 cmd)
{
	if(cmd > 0x7)
		cmd = 0x7;
	uint8 b = dt->tail;
	dt->raw[0] = (b & ~COMMAND_MASK) | ((cmd << 3) & COMMAND_MASK);
}


/**************************************************************************/
dacxx6x::channel_t::channel_t(dacxx6x &parent, uint8 ch_addr)
	: m_inst(parent)
{
	m_ch = ch_addr;
	m_gain = 1;
	m_enabled = false;
}

void dacxx6x::channel_t::setOutput(uint16 value, bool autoUpdateRegs)
{
	if(autoUpdateRegs)
		m_inst.write(value, m_ch, CMD_WRITE_UPDATE_IN_REG, false);
	else
		m_inst.write(value, m_ch, CMD_WRITE_IN_REG, false);
}

void dacxx6x::channel_t::setVoltage(float voltage, bool autoUpdateRegs)
{
	uint16 _dacValue = 0;
	voltage = fabsf(voltage);
	_dacValue = volts2dac(voltage);
	setOutput(_dacValue, autoUpdateRegs);
}

void dacxx6x::channel_t::setGain(GainMode gain)
{
	m_inst.write((uint16)gain, DAC_GAIN, CMD_WRITE_IN_REG);
	switch(gain)
	{
	case GainMode::INT_VREF:
		m_gain = 2;
		break;

	case GainMode::A1_B2:
		m_gain = (m_ch == DAC_A) ? 1 : 2;
		break;

	case GainMode::A2_B1:
		m_gain = (m_ch == DAC_A) ? 2 : 1;
		break;

	case GainMode::RESET:
		m_gain = 1;
		break;
	}
}

void dacxx6x::channel_t::update()
{
	// DAC's input register doesn't care of data bits when update command is used
	m_inst.write(0x0, m_ch, CMD_UPDATE_IN_REG);
}

void dacxx6x::channel_t::enable()
{
	switch(m_ch)
	{
	case DAC_A:
		m_inst.write(PwrUp::CH_A, 0x0, CMD_SET_POWER_MODE);
		m_enabled = true;
		break;

	case DAC_B:
		m_inst.write(PwrUp::CH_B, 0x0, CMD_SET_POWER_MODE);
		m_enabled = true;
		break;

	case DAC_AB:
		m_inst.write(PwrUp::CH_BOTH, 0x0, CMD_SET_POWER_MODE);
		m_enabled = true;
		break;

	default:
		m_enabled = false;
	}
}

void dacxx6x::channel_t::disable()
{
	m_inst.write((uint16)m_inst.m_powerDownMode, 0x0, CMD_SET_POWER_MODE);
	m_enabled = false;
}

bool dacxx6x::channel_t::isEnable() const
{
	return m_enabled;
}

float dacxx6x::channel_t::dac2volts(uint16 val)
{
	return ((val / 16383.0f) * m_inst.m_vref * (float)m_gain);
}

uint16 dacxx6x::channel_t::volts2dac(float vout)
{
	return ((vout * 16383.0f) / (m_inst.m_vref * m_gain));
}


/**************************************************************************/
const float dacxx6x::m_intVref = 2.5f;

#ifdef ARDUINO
SPISettings dacxx6x::m_spiSettings;
#else

// TODO: ESP IDF version

#endif

/**************************************************************************/
dacxx6x::dacxx6x()
{
// Note: This condition is used ONLY to determine if code is compiled with an Arduino API.
#ifdef ARDUINO
	m_spiDev = new SPIClass(VSPI);
	m_spiSettings = SPISettings(1000000UL, SPI_MSBFIRST, SPI_MODE0);

	m_spiMosi = -1;
	m_spiSck = -1;
	m_spiCs = -1;

	m_pinLdac = -1;

	m_vref = m_intVref;
#else
	// TODO: ESP IDF version
#endif
	// Channel objects init
	ch_a = new channel_t(*this, DAC_A);
	ch_b = new channel_t(*this, DAC_B);
}

dacxx6x::~dacxx6x()
{
	if(m_spiDev)
		m_spiDev->end();

	delete ch_a;
	delete ch_b;
}

void dacxx6x::init(int8 mosi, int8 sck, int8 cs, uint32_t clock)
{
#ifdef ARDUINO
	if(mosi == -1 && sck == -1 && cs == -1)
	{
		m_spiMosi = MOSI;
		m_spiSck = SCK;
		m_spiCs = SS;
	}
	else
	{
		m_spiMosi = mosi;
		m_spiSck = sck;
		m_spiCs = cs;
	}
	pinMode(m_spiCs, OUTPUT);
	digitalWrite(m_spiCs, HIGH); // for redundancy, cuz i dont belive in Arduino API
	m_spiSettings._clock = clock;
	m_spiDev->begin(m_spiSck, -1, m_spiMosi, m_spiCs);
#else
	// TODO: ESP IDF version
#endif
	// Set default configuration
	restoreDefault();
	delay(1);
}

void dacxx6x::setVref(float vref)
{
	if(vref < 0.0f)
		vref = 0.0f;
	if(vref > 5.0f)
		vref = 5.0f;
	m_vref = vref;
}

void dacxx6x::setIntRef(VrefCtrl mode)
{
	write((uint16)mode, 0x0, CMD_INT_REF_PWR);

	if (mode == VrefCtrl::ENABLE)
	{
		m_vref = m_intVref;
		ch_a->setGain(GainMode::INT_VREF);
		ch_b->setGain(GainMode::INT_VREF);
	}
	else if (mode == VrefCtrl::DISABLE)
	{
		ch_a->setGain(GainMode::RESET);
		ch_b->setGain(GainMode::RESET);
	}
}

void dacxx6x::attachLDAC(uint8 pin)
{
#ifdef ARDUINO
	m_pinLdac = pin;
	pinMode(m_pinLdac, OUTPUT);
	digitalWrite(m_pinLdac, HIGH);
#else
	// TODO: ESP IDF version

#endif
}

void dacxx6x::detachLDAC()
{
	if(m_pinLdac != -1)
	{
#ifdef ARDUINO
		digitalWrite(m_pinLdac, LOW);
		m_pinLdac = -1;
#else
		// TODO: ESP IDF version

#endif
	}
}

void dacxx6x::attachClear(uint8 pin)
{
#ifdef ARDUINO
	m_pinClr = pin;
	pinMode(m_pinClr, OUTPUT);
	digitalWrite(m_pinClr, HIGH); // internal pull-up
#else
	// TODO: ESP IDF version
	
#endif
}

void dacxx6x::detachClear()
{
	if(m_pinClr != -1)
	{
#ifdef ARDUINO
		digitalWrite(m_pinClr, LOW);
		m_pinClr = -1;
#else
		// TODO: ESP IDF version
#endif
	}
}

void dacxx6x::setLDAC(LdacCtrl mode)
{
	write((uint16)mode, 0x0, CMD_SET_LDAC_REGS);
}

void dacxx6x::updateAsync()
{
	if(m_pinLdac != -1)
	{
		// Just pulse pin
#ifdef ARDUINO
		digitalWrite(m_pinLdac, LOW);
		digitalWrite(m_pinLdac, HIGH);
#else
	// TODO: ESP IDF version

#endif
	}
}

void dacxx6x::setPowerDownMode(PwrDownMode mode)
{
	m_powerDownMode = (uint8)mode;
}

void dacxx6x::resetInputRegs()
{
	write((uint16)RstMode::DACS_ONLY, 0x0, CMD_RST);
}

void dacxx6x::factoryReset()
{
	write((uint16)RstMode::ALL, 0x0, CMD_RST);
}

void dacxx6x::hardwareClear()
{
	if(m_pinClr != -1)
	{
#ifdef ARDUINO
		digitalWrite(m_pinClr, LOW);
		digitalWrite(m_pinClr, HIGH);
#else	
		// TODO: ESP IDF version

#endif
	}
}

void dacxx6x::restoreDefault()
{
	setPowerDownMode(PwrDownMode::A_B_1K);	// both channels will be pulled-down with internal 1k resistor when disabling
	ch_a->disable();						// power down both channels
	ch_b->disable();
	setLDAC(LdacCtrl::NONE);				// LDAC disabled on both channels
	setIntRef(VrefCtrl::ENABLE);			// internal vref enabled
}

/**************************************************************************/
dac8162::dac8162() { }

dac8162::~dac8162() { }

DataFrame dac8162::write(uint16 data, uint8 address, uint8 command, bool sendingConfig)
{
	// Create 24-bit dataframe and pack all data into it
	DataFrame _dt = {0};
	_dt.bitOffset = 2;

	if (sendingConfig)
		_dt.data = data;
	else
		packData(&_dt, data);
	packAddress(&_dt, address);
	packCmd(&_dt, command);

#ifdef ARDUINO
	m_spiDev->beginTransaction(m_spiSettings);
	digitalWrite(m_spiCs, LOW);
	m_spiDev->transferBytes(_dt.raw, NULL, sizeof(_dt.raw));
	digitalWrite(m_spiCs, HIGH);
	m_spiDev->endTransaction();
#else
	// TODO: ESP IDF version

#endif
	return _dt;
}
