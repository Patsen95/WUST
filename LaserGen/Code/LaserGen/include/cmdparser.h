#pragma once

#include <Arduino.h>

#include "util.h"


// Max size of serial input buffer (for parsing)
#define IN_BUF_SIZE		20	// bytes

// Parameter name as offsets in data array received from serial port
#define _CMD			0
#define _SIG			1
#define _VALUE1			2
#define _VALUE2			3

#define FRAME_SIZE		4


typedef struct cmdframe_t
{
	char* _cmd;
	char* _sig;
	float _value1;
	float _value2;
};

typedef enum ParsingMode { RAW = 0, TEXT };

class CmdParser
{
public:
	CmdParser(ParsingMode mode = ParsingMode::TEXT);

	void parse(char* buf, size_t len);
	
	char* getParam(uint8 param);
	float getValue(uint8 valParam);
	cmdframe_t getComFrame();

private:
	ParsingMode m_parsingMode;
	char* m_tokens[FRAME_SIZE];
	cmdframe_t m_theframe;

	void reprint();
};
