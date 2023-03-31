/*
	ESP32 power monitor.
	Patryk Sienkiewcz @ WUST W12N, 2023
*/

// WiFi config
#define NETWORK_SSID	"TP-LINK_55708A"
#define NETWORK_PASS	"18333633"

// Adafruit IO config
#define AIO_SERVER		"io.adafruit.com"
#define IO_USERNAME		"Patsen95"
#define IO_KEY			"aio_dmKD84Z9L7fMDRaVKWfVQV0CuyT2"

#define USE_SSL			0
#define USE_SERIAL		0

#if USE_SSL

#define Awdwd

#endif

#define DAQ_RATE		3000   // Refresh rate in ms

// Hardware
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
#define STATUS_LED		2


