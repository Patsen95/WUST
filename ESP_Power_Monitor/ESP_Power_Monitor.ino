/*
	ESP32 power monitor.
	Patryk Sienkiewcz @ WUST W12N, 2023

	Check 'Config.h' before upload.

*/


#include "esp_aio.h"

#include "esp_system.h" // Watchdog functionality


#include <Adafruit_ST7789.h>
#include <Adafruit_GFX.h>


#define TO_VOLTS(val) ((float)(val * (3.3 / 4096)))


// Setup LCD
Adafruit_ST7789 tft = Adafruit_ST7789(LCD_CS, LCD_DC, LCD_MOSI, LCD_SCLK, LCD_RST);
GFXcanvas16 canvas(135, 240);

// Setup AIO connection object and remote variables
ESP_AIO_Client aio(NETWORK_SSID, NETWORK_PASS, IO_USERNAME, IO_KEY);

AIO_Publish *pub_SensAC = aio.makePublisher("/feeds/esp32-pwrmonitor.sens-ac");
AIO_Publish *pub_Sens12 = aio.makePublisher("/feeds/esp32-pwrmonitor.sens-12");
AIO_Publish *pub_Sens5 = aio.makePublisher("/feeds/esp32-pwrmonitor.sens-5");
AIO_Publish *pub_Sens33 = aio.makePublisher("/feeds/esp32-pwrmonitor.sens-33");

AIO_Subscribe *sub_RelAC = aio.makeSubscriber("/feeds/esp32-pwrmonitor.rel-ac");
AIO_Subscribe *sub_Rel12 = aio.makeSubscriber("/feeds/esp32-pwrmonitor.rel-12");
AIO_Subscribe *sub_Rel5 = aio.makeSubscriber("/feeds/esp32-pwrmonitor.rel-5");
AIO_Subscribe *sub_Rel33 = aio.makeSubscriber("/feeds/esp32-pwrmonitor.rel-33");

// Data variables
// Raw values from sensors
float s_ac;
float s_12;
float s_5;
float s_33;

// Remote button states (default LOW)
bool rel_ac_state = false;
bool rel_12_state = false;
bool rel_5_state = false;
bool rel_33_state = false;

// Function predefs
void onRel_AC(char *data, uint16_t len);
void onRel_12(char *data, uint16_t len);
void onRel_5(char *data, uint16_t len);
void onRel_33(char *data, uint16_t len);

void DisplayData();


void setup() 
{
	// Setup onborad LED (connection status LED)
	pinMode(STATUS_LED, OUTPUT);
	digitalWrite(STATUS_LED, LOW);

	// Setup serial connection
	Serial.begin(9600);
	while(!Serial);
	Serial.println();

	// Initialize LCD and draw static elements
	pinMode(LCD_BL, OUTPUT);
	digitalWrite(LCD_BL, HIGH); // Enable LCD backlight
	tft.init(135, 240);			// Initialize ST7789 240x135
	tft.fillScreen(ST77XX_BLACK);
	Serial.println("LCD initialized");

	// Setup data subscription object
	sub_RelAC->setCallback(onRel_AC);
	sub_Rel12->setCallback(onRel_12);
	sub_Rel5->setCallback(onRel_5);
	sub_Rel33->setCallback(onRel_33);

	aio.getMQTTClient()->subscribe(sub_RelAC);
	aio.getMQTTClient()->subscribe(sub_Rel12);
	aio.getMQTTClient()->subscribe(sub_Rel5);
	aio.getMQTTClient()->subscribe(sub_Rel33);

	DisplayData();

	// Connect to Adafruit IO
	aio.connect();

	Serial.println();
	Serial.println(aio.statusString());

}

uint16_t _lastTime = 0;
uint16_t _endTime = 0;

void loop() 
{
	uint16_t _time = millis();

	// We need to ping AIO server (kinda like TTL server pinging)
	// NOTE: use it only if you are publishing once evry KEEPALIVE seconds (~5 mins)
	// aio.ping();

	aio.getMQTTClient()->processPackets(1000);

	if(!aio.netConnected())
	{
		_endTime = millis();
		uint16_t _elapsed = _endTime - _lastTime;
		Serial.println(_elapsed);
		if (_elapsed >= 3000)
			esp_restart();
	}

	// Update display
	DisplayData();

	// Adafruit IO has minimum feed rate limit for publishing data
	// so some delay is required
	delay(DAQ_RATE);
}

void onRel_AC(char *data, uint16_t len)
{
	if (rel_ac_state == false)
		rel_ac_state = true;
	else
		rel_ac_state = false;
}

void onRel_12(char *data, uint16_t len)
{
	if (rel_12_state == false)
		rel_12_state = true;
	else
		rel_12_state = false;
}

void onRel_5(char *data, uint16_t len)
{
	if (rel_5_state == false)
		rel_5_state = true;
	else
		rel_5_state = false;
}

void onRel_33(char *data, uint16_t len)
{
	if (rel_33_state == false)
		rel_33_state = true;
	else
		rel_33_state = false;
}

void DisplayData()
{
	canvas.fillScreen(ST77XX_BLACK);
	canvas.setCursor(0, 7);
	canvas.setTextColor(ST77XX_WHITE);
	canvas.setTextSize(1);
	canvas.print("Status:");
	canvas.setCursor(50, 7);
	canvas.setTextColor(ST77XX_CYAN);
	if (aio.hostConnected())
	{
		canvas.setTextColor(ST77XX_GREEN);
		canvas.print("Connected");
	}
	else
	{
		canvas.setTextColor(ST77XX_CYAN);
		canvas.print("Not connected");
	}

	// Relays info
	canvas.drawLine(0, 25, tft.width(), 25, ST77XX_ORANGE);
	canvas.setTextColor(ST77XX_YELLOW);
	canvas.setCursor(tft.width() / 2 - 35, 30);
	canvas.setTextSize(2);
	canvas.print("Relays");

	if (!rel_ac_state)
		canvas.setTextColor(ST77XX_RED);
	else
		canvas.setTextColor(ST77XX_GREEN);
	canvas.setCursor(20, 55);
	canvas.print("AC");

	if (!rel_12_state)
		canvas.setTextColor(ST77XX_RED);
	else
		canvas.setTextColor(ST77XX_GREEN);
	canvas.setCursor(tft.width() - 60, 55);
	canvas.print("12V");

	if (!rel_5_state)
		canvas.setTextColor(ST77XX_RED);
	else
		canvas.setTextColor(ST77XX_GREEN);
	canvas.setCursor(20, 85);
	canvas.print("5V");

	if (!rel_33_state)
		canvas.setTextColor(ST77XX_RED);
	else
		canvas.setTextColor(ST77XX_GREEN);
	canvas.setCursor(tft.width() - 65, 85);
	canvas.print("3.3V");

	// Sens info
	canvas.drawLine(0, 105, tft.width(), 105, ST77XX_ORANGE);
	canvas.setTextColor(ST77XX_YELLOW);
	canvas.setCursor(tft.width() / 2 - 25, 110);
	canvas.setTextSize(2);
	canvas.print("Sens");
	canvas.setTextColor(ST77XX_ORANGE);
	canvas.setCursor(5, 140);
	canvas.print("AC:");
	canvas.setCursor(5, 165);
	canvas.print("12V:");
	canvas.setCursor(5, 190);
	canvas.print("5V:");
	canvas.setCursor(5, 215);
	canvas.print("3.3V:");

	canvas.setTextColor(ST77XX_CYAN);
	canvas.setCursor(70, 140);
	canvas.print(s_ac);
	canvas.print("V");
	canvas.setCursor(70, 164);
	canvas.print(s_12);
	canvas.print("V");
	canvas.setCursor(70, 190);
	canvas.print(s_5);
	canvas.print("V");
	canvas.setCursor(70, 215);
	canvas.print(s_33);
	canvas.print("V");

	tft.drawRGBBitmap(0, 0, canvas.getBuffer(), canvas.width(), canvas.height());
}
