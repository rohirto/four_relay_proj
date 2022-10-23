#include <stdio.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h> 
#include <WiFiManager.h> 
#include <ArduinoOTA.h>
#include <WebSocketsClient.h>
#include <DNSServer.h>
#include "u_macros.h"
#ifdef RASP_MQTT
#include <PubSubClient.h>
#else
//#include <UbidotsESPMQTT.h>
#include <PubSubClient.h>
#endif
#include <GDBStub.h>

//Defines
//#define DEBUG   

#ifdef RASP_MQTT    /* If localhost or else Ubidots */
const char* mqtt_server ="192.168.1.149";     //Static raspberry Server
WiFiClient espClient;
PubSubClient client(espClient);
#else
//Ubidots client(TOKEN);
char* client_name = MQTT_CLIENT_NAME;
const char* mqtt_server ="industrial.api.ubidots.com";
char payload[700];
char topic[150];
ESP8266WiFiMulti WiFiMulti;
WiFiClient ubidots;
PubSubClient client(ubidots);
#endif
bool connected  = false;

/********Func Prototypes********************************************/
void callback(char*, byte*, unsigned int);  //MQTT callback func
void reconnect(void);     //reconnect to MQTT
void Relay_setup(void);
void mqtt_subscribe(void);

/****************Control Variables ******************************/

//Relay Pins
#define RELAY_PIN_1     12
#define RELAY_PIN_2     14
#define RELAY_PIN_3     5
#define RELAY_PIN_4     4


/************ Timers Variables ************************************/
unsigned long startMillis;  //Some global vaiable anywhere in program
unsigned long currentMillis;
volatile byte temp_humd_timer = 30;  // In 10 secs multiple //1 min timer
volatile byte temp_humd_timer_elapsed = false;
volatile byte occupancy_timer_elapsed = false;
volatile byte energy_timer_elapsed = false;
volatile byte ten_sec_counter = 0;
volatile byte occupancy_timer = 6;
volatile byte energy_timer = 6;   // 1 min

/************ NRF Variables ************************************/
/*************NRF24************************/
/*******************************************/
/************* Weater Data ********************/

/**********************************************/
//Energy data 



void setup() {
  Serial.begin(115200);
  gdbstub_init();
  #if DEBUG
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  #endif
  Serial.println("Booting");

  WiFiManager wifiManager;
  wifiManager.setConfigPortalTimeout(180);
  wifiManager.autoConnect("Flop ESP", "espflopflop");
  //wifiManager.setSTAStaticIPConfig(IPAddress(192,168,1,150), IPAddress(192,168,1,1), IPAddress(255,255,255,0)); // optional DNS 4th argument
  //wifiManager.resetSettings();    //Uncomment to reset the Wifi Manager

  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("My Switch");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";
    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  ArduinoOTA.begin();
  /**********************OTA Ends**********************************************************************/
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  /* MQTT Settings */
  #ifdef RASP_MQTT    //If localhost else Ubidots
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  SUBSCRIBE();
  #else
  //client.setClientName(client_name);
 client.setServer(mqtt_server, 1883);
 Serial.println("Server Set");
    client.setCallback(callback);
    Serial.println("Callback Set");
    mqtt_subscribe();
  //SUBSCRIBE();
  //Serial.println("Subscribe Set");
  #endif

  /* Basic Setup */
 
  Relay_setup();

  /* NRF Setup */


  //  //Timer start
  startMillis = millis();

}

void loop() {
   char temp_buff1[20];

  /* OTA stuff */
  ArduinoOTA.handle();
  /*****OTA Ends **************/

  /* Wifi Stuff */
   if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  connected = client.connected();
  if (!connected) {
    #ifdef RASP_MQTT
    reconnect();
    #else
    //client.reconnect();
    reconnect();
    mqtt_subscribe();
    #endif
  }
  client.loop();


  /********** Application Code ********************************/
  //NRF Stuff
 


   //Occupancy Flag Update 

  //PIR auto mode related stuff
 

  timer_function(); //Update Timers 
  /************** App Code ends *******************************/
}

void callback(char* topic, byte* payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  char temp_buff[10];
  for (int i = 0; i<10; i++)
  {
    temp_buff[i] = '\0';
  }
  for( int i = 0; i < length; i++)
  {
    temp_buff[i] = (char)payload[i];
    Serial.print((char)payload[i]);  
  }
  Serial.println();
  float f_value = atof(temp_buff);
   if(strcmp(topic,SWITCH_1_TOPIC) == 0)
  {
    if(f_value == 1)
    {
      //ON command
       digitalWrite(RELAY_PIN_1, LOW);
    }
    else
    {
      //Off command
      digitalWrite(RELAY_PIN_1, HIGH);
    }
  }
  else if(strcmp(topic,SWITCH_2_TOPIC) == 0)
  {
    if(f_value == 1)
    {
      //ON command
      digitalWrite(RELAY_PIN_2, LOW);
    }
    else
    {
      //Off command
      digitalWrite(RELAY_PIN_2, HIGH);
    }
  }
  else if(strcmp(topic,SWITCH_3_TOPIC) == 0)
  {
    if(f_value == 1)
    {
       //ON command
      digitalWrite(RELAY_PIN_3, LOW);
    }
    else
    {
		//Off command
      digitalWrite(RELAY_PIN_3, HIGH);
    }
  }
  else if(strcmp(topic,SWITCH_4_TOPIC) == 0)
  {
    if(f_value == 1)
    {
       //ON command
      digitalWrite(RELAY_PIN_4, LOW);
    }
    else
    {
		//Off command
      digitalWrite(RELAY_PIN_4, HIGH);
    }
  }

  //Test Blink
#if DEBUG
int i;
  for(i =0;i<5;i++)
  {
     digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
    delay(1000);                      // Wait for a second
    digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
  }
#endif
}
#ifdef RASP_MQTT
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client_xyzsdjf")) {    //This MQTT CLient ID needs to be Unique
      Serial.println("connected");
      // Subscribe
      SUBSCRIBE();
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
#else
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    
    // Attempt to connect
    if (client.connect(MQTT_CLIENT_NAME, TOKEN,"")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);
    }
  }
}
#endif
void timer_function()
{
  currentMillis = millis();  //get the current "time" (actually the number of milliseconds since the program started)
  if ( currentMillis - startMillis >= 10000)
  {
    startMillis = currentMillis;
    ten_sec_counter++;

    if ((ten_sec_counter % occupancy_timer) == 0)
    {
      occupancy_timer_elapsed = true;
    }
    if((ten_sec_counter % energy_timer) == 0)
    {
      energy_timer_elapsed = true;
    }
    if ((ten_sec_counter % temp_humd_timer) == 0) //test whether the period has elapsed
    {
      //temp_humd_timer_elapsed = true;
      ten_sec_counter = 0;  //IMPORTANT to save the start time of the current LED state.
    }
  }

}






void Relay_setup()
{
  //define relay pins and their state
  pinMode(RELAY_PIN_1, OUTPUT);
  digitalWrite(RELAY_PIN_1, HIGH);
  pinMode(RELAY_PIN_2, OUTPUT);
  digitalWrite(RELAY_PIN_2, HIGH);
  pinMode(RELAY_PIN_3, OUTPUT);
  digitalWrite(RELAY_PIN_3, HIGH);
  pinMode(RELAY_PIN_4, OUTPUT);
  digitalWrite(RELAY_PIN_4, HIGH);
  
}

#ifndef RASP_MQTT
void publish_data(char* device_label, char* variable_label, char* payload_data)
{
  sprintf(topic, "%s", ""); // Cleans the topic content
  sprintf(topic, "%s%s", "/v1.6/devices/", device_label);

  sprintf(payload, "%s", ""); //Cleans the payload
  sprintf(payload, "{\"%s\":", variable_label); // Adds the variable label   
  sprintf(payload, "%s {\"value\": %s", payload, payload_data); // Adds the value
  sprintf(payload, "%s } }", payload); // Closes the dictionary brackets

  client.publish(topic, payload);
  client.loop();
  delay(1000);
}
void mqtt_subscribe()
{
  //char *topicToSubscribe;
  sprintf(topic, "%s", "");
  sprintf(topic, "%s", SWITCH_1_TOPIC);
  client.subscribe(topic);
  sprintf(topic, "%s", "");
  sprintf(topic, "%s", SWITCH_2_TOPIC);
  client.subscribe(topic);
  sprintf(topic, "%s", "");
  sprintf(topic, "%s", SWITCH_3_TOPIC);
  client.subscribe(topic);
	sprintf(topic, "%s", "");
  sprintf(topic, "%s", SWITCH_4_TOPIC);
  client.subscribe(topic);
}
#endif
