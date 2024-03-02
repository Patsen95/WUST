/**
*	@file dacxx6x.h
*
*	Texas Instruments DACxx6x chips family consists of six, high-precision & low power, double D/A converters:
*	2x 12-bit, 2x 14-bit and 2x 16-bit.
*	Each has internal 2.5V, configurable bidirectional voltage reference and buffered output stage.
*	More detailed information can be found in official product documentation:
*	@see https://www.ti.com/lit/ds/symlink/dac7563.pdf
*
*
*	This library provides methods to easily communicate with these chips through SPI interface.
*
*	@author Patryk Sienkiewicz (@patsen95), 2023
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

#pragma once


// Need to distinguish an Arduino API from ESP-IDF framework 
#ifdef ARDUINO
#include <Arduino.h>
#include <SPI.h>
#else

// TODO: ESP-IDF libs

#endif

// Helper types
typedef signed char 	int8;
typedef signed short 	int16;
typedef unsigned char 	uint8;
typedef unsigned short 	uint16;


// Address
#define DAC_A							0x0		// Channel A
#define DAC_B							0x1		// Channel B
#define DAC_GAIN						0x2		// Channel's gain (only use with command CMD_WRITE_IN_REG)
#define DAC_AB		 					0x7		// Address both channels

// Commands
#define CMD_WRITE_IN_REG				0x0
#define CMD_UPDATE_IN_REG				0x1
#define CMD_WRITE_UPDATE_BOTH_IN_REGS	0x2
#define CMD_WRITE_UPDATE_IN_REG			0x3
#define CMD_SET_POWER_MODE				0x4
#define CMD_RST							0x5
#define CMD_SET_LDAC_REGS				0x6
#define CMD_INT_REF_PWR					0x7


/// @brief Gain modes.
enum GainMode
{
	INT_VREF	=	0x0,	// Gain = 2 on both channels  
	A1_B2		=	0x1,	// Ch A gain = 1, Ch B gain = 2
	A2_B1		=	0x2,	// Ch A gain = 2, Ch B gain = 1
	RESET		=	0x3		// Gain = 1 on both channels
};

/// @brief Power-up modes.
enum PwrUp
{
	CH_A		=	0x1,			
	CH_B		=	0x2,
	CH_BOTH		=	(CH_A | CH_B)
};

/// @brief Power-down modes.
enum PwrDownMode
{
	A_1K		=	0x11,				// Ch A, 1k pull-down
	B_1K		=	0x12,				// Ch B, 1k pull-down
	A_B_1K		=	(A_1K | B_1K),		// Ch A & B, 1k pull-down
	A_100K		=	0x21,				// Ch A, 100k pull-down
	B_100K		=	0x22,				// Ch B, 100k, pull-down
	A_B_100k	=	(A_100K | B_100K),	// Ch A & B, 100k pull-down
	A_HI_Z		=	0x31,				// Ch A, High-Z (floating)
	B_HI_Z		=	0x32,				// Ch B, High-Z (floating)
	A_B_HI_Z	=	(A_HI_Z | B_HI_Z)	// Ch A & B, High-Z (floating)
};

/// @brief Software reset modes for internal registers.
enum RstMode
{
	DACS_ONLY	=	0x0,	// Reset only data input regs
	ALL			=	0x1		// Reset all user-accessible regs
};

/// @brief LDAC control.
enum LdacCtrl
{
	A_AND_B		=	0x0,	// LDAC on both channels
	B_ONLY		=	0x1,	// LDAC on Ch B
	A_ONLY		=	0x2,	// LDAC on Ch A
	NONE		=	0x3		// LDAC off
};

/// @brief Internal VREF source control.
enum VrefCtrl
{
	DISABLE	= 	0x0,
	ENABLE 	=	0x1
};

// Dataframe masks
#define ADDRESS_MASK		0x07
#define COMMAND_MASK		0x38


#ifdef __cplusplus
extern "C" {
#endif
	/**
	 * @brief Implementation of data structure used to communicate with all Texas Instuments DACxx6x family.
	 * Representing an actuall data register format from chip's official documentation.
	 * Code features special functions dedicated to prepare valid data frame that later is send into DAC.
	 */
	typedef struct _dataFrame
	{
		union
		{
			uint8 raw[3]; 		// 24-bit array representing whole data frame in raw form
			struct
			{
				uint8 tail;		// Address & command (both are only 3 bits long so they perfecly fit in a single byte)
				uint16 data;	// Data section in frame is max 16-bit long
			};
		};

		uint8 bitOffset;		// Determines offset (shift) of data segment in Data Input Frame Register format, specific for each chip model (12-bit, 14-bit or 16-bit).
	} DataFrame;

	/// @brief Converts 16-bit value and packs it into referenced DataFrame object.
	/// @param dt Reference to DataFrame object.
	/// @param data Integer value to be packed.
	void packData(DataFrame *dt, uint16 data);

	/// @brief Converts address byte to fit lower 3-bit format in DataFrame's tail.
	/// @param dt Reference to DataFrame object.
	/// @param addr Address value to be packed.
	void packAddress(DataFrame *dt, uint8 addr);

	/// @brief Converts command byte to fit higher 3-bit format in DataFrame's tail.
	/// @param dt Reference to DataFrame object.
	/// @param cmd Command value to be packed.
	void packCmd(DataFrame *dt, uint8 cmd);

	/// @brief Function to obtain Data section from DataFrame valid object. 
	/// @param dt Const reference to DataFrame object.
	/// @return Converted internal value to 16-bit uint.
	inline uint16 unpackData(const DataFrame *dt)
	{
		return (dt->raw[1] << 8) | dt->raw[2];
	}

	/// @brief Funtion to obtain 3-bit address as an 8-bit uint.
	/// @param dt Const reference to DataFrame object.
	/// @return Converted internal 3-bit value to 8-bit uint.
	inline uint8 unpackAddress(const DataFrame *dt)
	{
		return dt->raw[0] & ADDRESS_MASK;
	}

	/// @brief Function to obtain 3-bit command as an 8-bit uint.
	/// @param dt Const reference to DataFrame object.
	/// @return Converted internal 3-bit balue to 8-bit uint.
	inline uint8 unpackCmd(const DataFrame *dt)
	{
		return (dt->raw[0] >> 3) & 0x7;
	}
#ifdef __cplusplus
}
#endif


/// @brief A base class that serves as an interface for operating all of the DACxx6x chips.
class dacxx6x
{
private:
	/// @brief Struct handling a single D/A channel.
	struct channel_t
	{
		channel_t(dacxx6x &parent, uint8 ch_addr);

		/// @brief Sets channel's output voltage from the given 16-bit integer value.
		/// @param value 16-bit integer representing output voltage
		/// @param autoUpdateRegs (Optional) If true, chip performs auto-update of it's internal input register. Default is true
		void setOutput(uint16 value, bool autoUpdateRegs = true);

		/// @brief Sets channel's output voltage directly from the given voltage value.
		/// @param voltage Output voltage float to be set
		/// @param autoUpdateRegs (Optional) If true, chip performs auto-update of it's internal input register. Default is true
		void setVoltage(float voltage, bool autoUpdateRegs = true);

		/// @brief Sets gain registry value for each channel.
		/// @param gain Selected gain mode
		void setGain(GainMode gain);

		/// @brief Allows to manualy update DAC's input register and set an output.
		void update();

		/// @brief Enables channel.
		void enable();

		/// @brief Disables channel with selected mode from setPowerDownMode()
		void disable();

		/// @brief Checks if channel is enabled.
		/// @return True, if enabled
		bool isEnable() const;

	private:
		/// @brief Instance of parent class (for access to it's fields).
		dacxx6x &m_inst;

		uint8 m_ch;
		uint8 m_gain;
		bool m_enabled;

		/// @brief Helper method used to calculate specific output voltage from 16-bit value (with Gain Transfer Formula from docs).
		/// @param val 16-bit value representing output voltage
		/// @return Voltage value
		float dac2volts(uint16 val);

		/// @brief Helper method used to calculate 16-bit value corresponding to the provided voltage (with Gain Transfer Formula from docs).
		/// @param vout Voltage value
		/// @return 16-bit voltage respresentaion
		uint16 volts2dac(float vout);
	};

public:
	dacxx6x();
	~dacxx6x();

	/// @brief Initializes library and configures SPI interface.
	/// @param mosi (Optional) SPI MOSI pin
	/// @param sck (Optional) SPI SCK pin
	/// @param cs (Optional) SPI Chip Select
	/// @param clock (Optional) SPI clocking speed (1 MHz by default)
	void init(int8 mosi = -1, int8 sck = -1, int8 cs = -1, uint32_t clock = 1000000UL);

	/// @brief Sets voltage reference value. Needed to properly calculate an output voltage.
	/// @note This method must be called after disabling DAC's internal VREF source.
	/// @param vref Voltage reference value
	void setVref(float vref);

	/// @brief Enable or disable
	/// @param mode
	void setIntRef(VrefCtrl mode);

	/// @brief Allows use of LDAC signal to make hardware-side data register update.
	/// @param pin Digtial output pin where LDAC is attached to
	void attachLDAC(uint8 pin);

	/// @brief Disables LDAC operation.
	void detachLDAC();

	/// @brief Allows to attach hardware clear signal.
	/// @note Pin is active-low. Pull-up is done internally.
	/// @param pin Digital output pin to CLR line
	void attachClear(uint8 pin);

	/// @brief Disables CLR line.
	void detachClear();

	/// @brief Set LDAC opeartion mode.
	/// @param mode Operation mode. LDAC can be activated for specific or both channels, or disabled
	void setLDAC(LdacCtrl mode);

	/// @brief Performs software immediate update of data register.
	void updateAsync();

	/// @brief Set power-down mode. DACxx6x family has ability to set specific power-down mode for each channel by single command.
	/// Channel can be disabled with one of the internal pull-down resistor or set as floating (high-Z).  
	/// @param mode Power-down mode
	void setPowerDownMode(PwrDownMode mode);

	/// @brief Performs software reset of data input register.
	void resetInputRegs();

	/// @brief Returns chip to it's factory state (clears all registers set by user).
	void factoryReset();

	/// @brief Returns chip to it's factory state through hardware pin.
	void hardwareClear();

	/// @brief Restores DAC to initial state provided by this library (same as init() method).
	void restoreDefault();

	/// @brief Reference to specific DAC channel.
	channel_t *ch_a, *ch_b;

protected:

#ifdef ARDUINO
	SPIClass *m_spiDev;
	static SPISettings m_spiSettings;
#else

		// TODO: ESP IDF version

#endif

	int8 m_spiMosi;
	int8 m_spiSck;
	int8 m_spiCs;

	int8 m_pinLdac;
	int8 m_pinClr;

	uint8 m_powerDownMode;
	float m_vref;
	static const float m_intVref;

	/// @brief Creates valid data frame from provided arguments and transmits it to DAC through SPI interface.
	/// @param data Data value segment
	/// @param address Address segment
	/// @param command Command segment
	/// @param sendingConfig (Optional) If true, doesn't use @see packData() function to convert 16-bit value to data input format. Default is true
	/// @return Copy of created DataFrame object
	virtual DataFrame write(uint16 data, uint8 address, uint8 command, bool sendingConfig = true) = 0;
};

/// @brief Model-specific definition of generalized dacxx6x object.
class dac8162 : public dacxx6x
{
public:
	dac8162();
	virtual ~dac8162();

private:
	DataFrame write(uint16 data, uint8 address, uint8 command, bool sendingConfig = true) override;
};
