/*
	ESP32 power monitor.
	Patryk Sienkiewcz @ WUST W12N, 2023

	CHECK CONFIG FILE ON START!!!
*/

#include "config.h"

#include "AdafruitIO_WiFi.h"


#define TO_VOLTS(val) ((float)(val * (3.3 / 4096)))


// Setup AIO connection object and remote variables
AdafruitIO_WiFi aio(IO_USERNAME, IO_KEY, NETWORK_SSID, NETWORK_PASS);
AdafruitIO_Group *group = aio.group("esp32-pwrmonitor");


struct _dpack
{
	// Raw values from sensors
	float s_ac; 
	float s_12; 
	float s_5; 
	float s_33;

	// Remote button states (default LOW)
	bool rel_ac_state;
	bool rel_12_state;
	bool rel_5_state;
	bool rel_33_state;
} _aio_packet;


void readSensors(void);


void setup() {

	// GPIO setup
	analogReadResolution(12);
	pinMode(STATUS_LED, OUTPUT);
	digitalWrite(STATUS_LED, LOW);

	// Start serial (debugging only)
#if USE_SERIAL
	Serial.begin(9600);
	while(!Serial);
	Serial.println("Connecting to Adafruit IO...");
#endif

	_aio_packet.rel_ac_state = false;
	_aio_packet.rel_12_state = false;
	_aio_packet.rel_5_state = false;
	_aio_packet.rel_33_state = false;

	// Connect to io.adafruit.com
	aio.connect();
	while(aio.status() < AIO_CONNECTED)
	{
#if USE_SERIAL
		Serial.print(".");
#endif
		delay(500);
	}

	// Register callback functions (to operate relays remotely)
	group->onMessage("rel-ac", onRel_AC);
	group->onMessage("rel-12", onRel_12);
	group->onMessage("rel-5", onRel_5);
	group->onMessage("rel-33", onRel_33);

#if USE_SERIAL
	Serial.println();
	Serial.println(aio.statusText());
#endif
}

void loop() {

	// Kinda TTL server pinging
	aio.run();

	// Read all inputs...
	readSensors();

	// ...display them...

	// ...and send it!
#if USE_SERIAL
	Serial.println("SENDING:");
	Serial.print("[RELAY_AC]: ");
	Serial.println(_aio_packet.s_ac);
	Serial.print("[RELAY_12]: ");
	Serial.println(_aio_packet.s_12);
	Serial.print("[RELAY_5]: ");
	Serial.println(_aio_packet.s_5);
	Serial.print("[RELAY_33]: ");
	Serial.println(_aio_packet.s_33);
	Serial.println("================");
#endif

	// Set status led HIGH when sending data
	//digitalWrite(STATUS_LED, HIGH);
	group->set("sens-ac", _aio_packet.s_ac);
	group->set("sens-12", _aio_packet.s_12);
	group->set("sens-5", _aio_packet.s_5);
	group->set("sens-33", _aio_packet.s_33);
	group->save();
	//digitalWrite(STATUS_LED, LOW);

	// Adafruit IO has minimum feed rate limit for publishing data
	// so some delay is required
	delay(DAQ_RATE);
}

void readSensors(void)
{
	_aio_packet.s_ac = TO_VOLTS(analogRead(SENS_AC)) * 100;
	_aio_packet.s_12 = TO_VOLTS(analogRead(SENS_12));
	_aio_packet.s_5 = TO_VOLTS(analogRead(SENS_5));
	_aio_packet.s_33 = TO_VOLTS(analogRead(SENS_33));
}

// =================================================================
// Callback functions to process received data from certain feeds
void onRel_AC(AdafruitIO_Data *data)
{
	_aio_packet.rel_ac_state = (data->toString() == "ON" ? true : false);
	digitalWrite(STATUS_LED, _aio_packet.rel_ac_state);

#if USE_SERIAL
	Serial.print("received [RELAY_AC]: ");
	Serial.println((_aio_packet.rel_ac_state ? "ON" : "OFF"));
#endif
}

void onRel_12(AdafruitIO_Data *data)
{
	_aio_packet.rel_12_state = (data->toString() == "ON" ? true : false);
	digitalWrite(STATUS_LED, _aio_packet.rel_12_state);

#if USE_SERIAL
	Serial.print("received [RELAY_AC]: ");
	Serial.println((_aio_packet.rel_12_state ? "ON" : "OFF"));
#endif
}

void onRel_5(AdafruitIO_Data *data)
{
	_aio_packet.rel_5_state = (data->toString() == "ON" ? true : false);
	digitalWrite(STATUS_LED, _aio_packet.rel_5_state);

#if USE_SERIAL
	Serial.print("received [RELAY_AC]: ");
	Serial.println((_aio_packet.rel_5_state ? "ON" : "OFF"));
#endif
}

void onRel_33(AdafruitIO_Data *data)
{
	_aio_packet.rel_33_state = (data->toString() == "ON" ? true : false);
	digitalWrite(STATUS_LED, _aio_packet.rel_33_state);

#if USE_SERIAL
	Serial.print("received [RELAY_AC]: ");
	Serial.println((_aio_packet.rel_33_state ? "ON" : "OFF"));
#endif
}