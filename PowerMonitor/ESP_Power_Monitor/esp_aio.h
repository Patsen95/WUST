/*
	Wrapper for an AdafruitIO and AdafruitMQTT libraries.
	Implementated for WiFi but can be easly adaptet for Ethernet or other.

	Adapted for custom, specific usage!!!

	Copyright Patryk Sienkiewcz @ WUST, 2023

	******************************************************************************************
	The MIT License (MIT)

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#ifndef ESP_AIO_H
#define ESP_AIO_H


#include "config.h"

#include "Arduino.h"
#include "WiFiClientSecure.h"
#include "Adafruit_MQTT_Client.h"
#include "AdafruitIO_Definitions.h"



#if (USE_SERIAL == 1)
#define DPRINT(args...) ({ printf(args); })
#else
#define DPRINT(...) ({})
#endif


#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)

// Forward decl
class Adafruit_MQTT_Publish;
class Adafruit_MQTT_Subscribe;
class AIO_Feed;
class AIO_Group;

typedef Adafruit_MQTT_Publish 		AIO_Publish;
typedef Adafruit_MQTT_Subscribe 	AIO_Subscribe;
// typedef AdafruitIO_Feed 			AIO_Feed;
// typedef AdafruitIO_Group 			AIO_Group;

// ############################################################################
/*!
	@brief  Class that provides methods for simplest possible interfacing
			with Adafruit IO protocol.
			This interface directly supports WiFi connection.

	@warning Check the "config.h" file before attempting the connection.
*/
class ESP_AIO_Client
{
public:
	ESP_AIO_Client(const char* ssid, const char* pass, const char* user, const char* key);
	~ESP_AIO_Client();

	virtual void connect();
	virtual void disconnect();
	bool ping();
	

	Adafruit_MQTT_Client* getMQTTClient() const;
	aio_status_t getStatus() const;
	const char* statusString() const;
	bool hostConnected() const;
	bool netConnected();


	AIO_Publish* makePublisher(const char* path);
	AIO_Subscribe* makeSubscriber(const char* path);

	// TODO: Implement appropriate classes for simpler Feed & Group handling
	// AIO_Feed* attachFeed(const char* path);
	// AIO_Group* attachGroup(const char* path);


protected:
	const char* m_ssid;
	const char* m_password;
	const char* m_username;
	const char* m_key;
	const char* m_host = AIO_SERVER;

	aio_status_t m_status;

	WiFiClientSecure* m_client;
	Adafruit_MQTT_Client* m_mqtt_client;

	void _initNet();


private:
	const uint16_t PORT_SECURE = 8883;
	const uint8_t MAX_CONN_ATTEMPTS = 10;
	const uint16_t RETRY_DELAY = 500;

	char* m_pub_topic;
	char* m_sub_topic;

	// SSL certificate
	const char* m_aio_ca =
		"-----BEGIN CERTIFICATE-----\n"
		"MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n"
		"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"
		"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n"
		"QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n"
		"MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n"
		"b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n"
		"9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\n"
		"CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\n"
		"nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\n"
		"43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\n"
		"T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\n"
		"gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\n"
		"BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\n"
		"TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\n"
		"DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\n"
		"hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\n"
		"06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\n"
		"PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\n"
		"YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\n"
		"CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\n"
		"-----END CERTIFICATE-----\n";

	bool m_isConnected;
	bool m_netEstablished;
	
	aio_status_t _netStatus(wl_status_t net_status = WiFi.status());
};


#else 
#error "Library designed ONLY for ESP32!"
#endif
#endif // ESP_AIO_H
