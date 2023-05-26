

#include "esp_aio.h"



ESP_AIO_Client::ESP_AIO_Client(const char* ssid, const char* pass, const char* user, const char* key)
{
	m_ssid = ssid;
	m_password = pass;
	m_username = user;
	m_key = key;
	m_netEstablished = false;
	m_isConnected = false;
	m_status = AIO_NET_DISCONNECTED;
	m_client = new WiFiClientSecure();
	m_mqtt_client = new Adafruit_MQTT_Client(m_client, m_host, PORT_SECURE, m_username, m_key);
	m_pub_topic = nullptr;
	m_sub_topic = nullptr;
}

ESP_AIO_Client::~ESP_AIO_Client()
{
	if(m_netEstablished)
		disconnect();

	if(m_client)
		delete m_client;

	if(m_mqtt_client)
		delete m_mqtt_client;

	if(m_pub_topic)
		free(m_pub_topic);

	if(m_sub_topic)
		free(m_sub_topic);
}

void ESP_AIO_Client::connect()
{
	DPRINT("[CONNECT] ");

	_initNet();

	if(m_isConnected)
	{
		DPRINT("Already connected to %s!\n", m_host);
		return;
	}
	if(m_netEstablished)
	{
		DPRINT("Attempting connect to %s...\n", m_host);
		uint8_t _attempts = MAX_CONN_ATTEMPTS / 2;
		int8_t _ret;
		while((_ret = m_mqtt_client->connect()) != 0)
		{
			DPRINT("Atempt %d\n", _attempts);
			DPRINT("MQTT error: %s\n", m_mqtt_client->connectErrorString(_ret));
			m_mqtt_client->disconnect();
			delay(RETRY_DELAY * 6);
			if(_attempts <= 0)
			{
				DPRINT("Failed to connect to the host: %s\n", m_host);
				m_mqtt_client->disconnect();
				m_status = AIO_CONNECT_FAILED;
				return;
			}
			_attempts--;
		}
		m_status = AIO_CONNECTED;
		m_isConnected = true;
		DPRINT("MQTT connected!\n");
	}
	else
		DPRINT("Can't connect to MQTT - internet connection not established!");
}

void ESP_AIO_Client::disconnect()
{
	DPRINT("[DISCONNECT] ");
	if(m_isConnected)
	{
		DPRINT("Disconnecting from HOST: %s...\n", m_host);
		m_mqtt_client->disconnect();
		m_isConnected = false;
		DPRINT("Disconnected from host!\n");
	}
	DPRINT("Disconnecting from network: %s...\n", m_ssid);
	WiFi.disconnect();
	delay(AIO_NET_DISCONNECT_WAIT);
	m_status = _netStatus();
	m_netEstablished = false;
	DPRINT("Disconnected from network!\n");
}

bool ESP_AIO_Client::ping()
{
	if(m_isConnected)
	{
		if(m_mqtt_client->ping())
			return true;
		else
		{
			DPRINT("[PING] Error: No response from server!\n");
			return false;
		}
	}
	return false;
}


Adafruit_MQTT_Client* ESP_AIO_Client::getMQTTClient() const
{
	return m_mqtt_client;
}

aio_status_t ESP_AIO_Client::getStatus() const
{
	return m_status;
}

const char* ESP_AIO_Client::statusString() const
{
	switch(m_status)
	{
		case AIO_IDLE:
			return "Waiting for connect to be called...";
		case AIO_NET_DISCONNECTED:
			return "Network disconnected.";
		case AIO_DISCONNECTED:
			return "Disconnected from Adafruit IO.";

		case AIO_NET_CONNECT_FAILED:
			return "Network connection failed.";
		case AIO_CONNECT_FAILED:
			return "Adafruit IO connection failed.";
		case AIO_FINGERPRINT_INVALID:
			return "Adafruit IO SSL fingerprint verification failed.";
		case AIO_AUTH_FAILED:
			return "Adafruit IO authentication failed.";

		case AIO_NET_CONNECTED:
			return "Network connected.";
		case AIO_CONNECTED:
			return "Adafruit IO connected (SSL/TLS).";
		case AIO_CONNECTED_INSECURE:
			return "Adafruit IO connected.";
		case AIO_FINGERPRINT_UNSUPPORTED:
			return "Adafruit IO connected over SSL/TLS. Fingerprint verification unsupported.";
		case AIO_FINGERPRINT_VALID:
			return "Adafruit IO connected over SSL/TLS. Fingerprint valid.";

		default:
			return "Unknown status code!";
	}
}

bool ESP_AIO_Client::netConnected()
{
	m_netEstablished = (_netStatus() != AIO_NET_DISCONNECTED);
	return m_netEstablished;
}

bool ESP_AIO_Client::hostConnected() const
{
	return (bool)(m_status >= AIO_CONNECTED);
}

AIO_Publish* ESP_AIO_Client::makePublisher(const char *path)
{
	if(!path)
		return nullptr;

	m_pub_topic = (char*)malloc(sizeof(char) * (strlen(m_username) + strlen(path) + 1));
	strcpy(m_pub_topic, m_username);
	strcat(m_pub_topic, path);
	return new AIO_Publish(m_mqtt_client, m_pub_topic);
}

AIO_Subscribe* ESP_AIO_Client::makeSubscriber(const char *path)
{
	if(!path)
		return nullptr;

	m_sub_topic = (char*)malloc(sizeof(char) * (strlen(m_username) + strlen(path) + 1));
	strcpy(m_sub_topic, m_username);
	strcat(m_sub_topic, path);
	return new AIO_Subscribe(m_mqtt_client, m_sub_topic);
}

// TODO: Implement interface for handling Feed & Group topics
// AIO_Feed* ESP_AIO_Client::attachFeed(const char *feedName)
// {
// 	if(!feedName)
// 		return nullptr;
//
// 	return nullptr;
// }

// AIO_Group* ESP_AIO_Client::attachGroup(const char *groupName)
// {
// 	if (!groupName)
// 		return nullptr;
//
// 	return nullptr;
// }

void ESP_AIO_Client::_initNet()
{
	DPRINT("[NET INIT] ");
	if(m_netEstablished)
		return;
	
	if(strlen(m_ssid) == 0)
	{
		m_status = AIO_SSID_INVALID;
		DPRINT("Error: %s\n", statusString());
	}
	else
	{
		DPRINT("Connecting to SSID: %s...\n", m_ssid);
		uint8_t _attempts = MAX_CONN_ATTEMPTS;

		WiFi.begin(m_ssid, m_password);

		while (_netStatus() != AIO_NET_CONNECTED)
		{
		DPRINT("Atempt %d\n", _attempts);
		if (_attempts <= 0)
		{
			DPRINT("Failed to connect to WiFi!\n");
			WiFi.disconnect(); // Also resets device (setting default state)
			m_status = AIO_NET_CONNECT_FAILED;
			return;
		}
		else
			_attempts--;
		delay(RETRY_DELAY);
		}
		m_status = _netStatus();
		m_netEstablished = true;
		m_client->setCACert(m_aio_ca);
		DPRINT("WiFi connection established!\n");
		DPRINT("Current IP: %s\n", WiFi.localIP().toString().c_str());
	}
	DPRINT("Status: %s\n", statusString());
}

aio_status_t ESP_AIO_Client::_netStatus(wl_status_t net_status)
{
	switch(net_status)
	{
	case WL_CONNECTED:
		return AIO_NET_CONNECTED;
	case WL_CONNECT_FAILED:
		return AIO_NET_CONNECT_FAILED;
	case WL_IDLE_STATUS:
		return AIO_IDLE;

	default:
		return AIO_NET_DISCONNECTED;
	}
}
