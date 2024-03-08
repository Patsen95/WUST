#include "cmdparser.h"


#define to_uint(x)		((uint16)atoi(x))
#define to_float(x)		((float)atoff(x))


CmdParser::CmdParser(ParsingMode mode)
	: m_parsingMode(mode) 
	{
		m_theframe =
			{
				._cmd = "",
				._sig = "",
				._value1 = -1.0f,
				._value2 = -1.0f
			};
	}

void CmdParser::parse(char* buf, size_t len)
{
	char *token;
	char *inputStr = buf;
	uint8 idx = 0;

	// Tokenize input string
	while((token = strtok_r(inputStr, " ", &inputStr))) // space as separator
	{
		if(idx >= FRAME_SIZE)
			idx = 0;

		m_tokens[idx] = token;
		idx++;
	}

	switch(m_parsingMode)
	{
		case ParsingMode::RAW:

			break;

		case ParsingMode::TEXT:

			char* _cmd = m_tokens[_CMD];
			char* _sig = m_tokens[_SIG];
			char* _val1 = m_tokens[_VALUE1];
			char *_val2 = m_tokens[_VALUE2];

			if(!strcmp(_sig, "sin") || !strcmp(_sig, "saw"))
				m_theframe._sig = _sig;

			if(!strcmp(_cmd, "en"))
			{
				m_theframe._cmd = _cmd;
				m_theframe._value2 = -1.0f;

				if(!strcmp(_val1, "t\n"))
					m_theframe._value1 = 1.0f;
				else if(!strcmp(_val1, "f\n"))
					m_theframe._value1 = 0.0f;
				else
					m_theframe._value1 = -1.0f;
			}

			if(!strcmp(_cmd, "amp"))
			{
				m_theframe._cmd = _cmd;
				m_theframe._value1 = to_float(_val1);
				m_theframe._value2 = -1.0f;
			}

			if(!strcmp(_cmd, "freq"))
			{
				m_theframe._cmd = _cmd;
				m_theframe._value1 = to_float(_val1);
				m_theframe._value2 = -1.0f;
			}

			if(!strcmp(_cmd, "ph"))
			{
				m_theframe._cmd = _cmd;
				m_theframe._value1 = to_float(_val1);
				m_theframe._value2 = -1.0f;
			}

			if(!strcmp(_cmd, "dc"))
			{
				m_theframe._cmd = _cmd;
				m_theframe._value1 = to_float(_val1);
				m_theframe._value2 = -1.0f;
			}

			if(!strcmp(_cmd, "swe"))
			{
				m_theframe._cmd = _cmd;
				m_theframe._value2 = -1.0f;

				if (!strcmp(_val1, "t\n"))
					m_theframe._value1 = 1.0f;
				else if (!strcmp(_val1, "f\n"))
					m_theframe._value1 = 0.0f;
				else
					m_theframe._value1 = -1.0f;
			}

			if(!strcmp(_cmd, "swp"))
			{
				m_theframe._cmd = _cmd;
				m_theframe._value1 = to_float(_val1);
				m_theframe._value2 = to_float(_val2);
			}

			if(!strcmp(_cmd, "swr"))
			{
				m_theframe._cmd = _cmd;
				m_theframe._value1 = to_float(_val1);
				m_theframe._value2 = -1.0f;
			}

			if(!strcmp(_cmd, "swf"))
			{
				m_theframe._cmd = _cmd;
				m_theframe._value1 = to_float(_val1);
				m_theframe._value2 = -1.0f;
			}
			break;
	}
	reprint();
}

char* CmdParser::getParam(uint8 param)
{
	if(param < 0 || param > FRAME_SIZE)
		return NULL;

	return m_tokens[param];
}

float CmdParser::getValue(uint8 valParam)
{
	switch(valParam)
	{
		case _VALUE1:
			return to_float(m_tokens[_VALUE1]);
			break;
		
		case _VALUE2:
			return to_float(m_tokens[_VALUE2]);
			break;
		
		default:
			return -1.0f;
	}
}

cmdframe_t CmdParser::getComFrame()
{
	return m_theframe;
}

void CmdParser::reprint()
{
	Serial.println(m_theframe._cmd);
	Serial.println(m_theframe._sig);
	Serial.println(m_theframe._value1);
	Serial.println(m_theframe._value2);
}
