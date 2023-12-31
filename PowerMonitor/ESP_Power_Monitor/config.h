/*
	ESP32 power monitor.
	Patryk Sienkiewcz @ WUST W12N, 2023
*/

#define ESP32 					// Theoretically defined by IDE but who knows...

//*********************	WIFI CONFIG *********************//
#define NETWORK_SSID	"PATSENPC_NET"
#define NETWORK_PASS	"patsen95"

//********************* ADAFRUIT IO CONFIG *********************//
#define AIO_SERVER		"io.adafruit.com"
#define IO_USERNAME		"Patsen95"
#define IO_KEY 			"aio_omqe68wVLExwz3UfvDjBRO0UvHb1"

//********************* SYSTEM CONFIG *********************
#define USE_SERIAL		1		// Debug / logging through default serial
#define USE_LCD			1
#define DISPLAY_STATUS	1		// 1 - display status on LCD (if attached)

#define THROTTLE_DELAY	3000	// Minimum delay to satisfy AIO throttle limit.
								// Should be set accordingly to amount of sent variables.


//********************* HARDWARE *********************//
#if (USE_LCD == 1)
// Display GPIO
#define LCD_MOSI 	19
#define LCD_SCLK 	18
#define LCD_CS		5
#define LCD_DC 		16
#define LCD_RST 	23
#define LCD_BL 		4
#endif

// Output relays GPIO
#define REL_AC
#define REL_12
#define REL_5
#define REL_33

// Voltage sensing GPIO
#define SENS_AC			33
#define SENS_12			32
#define SENS_5			35
#define SENS_33			34

// Status LED GPIO
// #define STATUS_LED		2
