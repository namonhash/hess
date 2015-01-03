#include <SPI.h>
#include <Ethernet.h>
#include "AirQuality.h"
#include "Arduino.h"
#include "DHT.h"
#include "HttpClient.h"
#include <avr/wdt.h>
#include <Wire.h>
//#include <SeeedGrayOLED.h>
#include <avr/pgmspace.h>
#include <PubSubClient.h>
#include <allthingstalk_arduino_standard_lib.h>

#define airquality_sensor_pin A0
#define sound_sensor_pin A1
#define light_sensor_pin A2
#define gas_sensor_pin A3
#define DHTPIN 7 
#define DHTTYPE DHT22 
DHT dht(DHTPIN, DHTTYPE); // Initialize DHT object

char deviceId[] = "Your device id comes here";
char clientId[] = "Your client id comes here"; 
char clientKey[] = "Your client key comes here";

ATTDevice Device(deviceId, clientId, clientKey);            //create the object that provides the connection to the cloud to manager the device.
char httpServer[] = "api.smartliving.io";                  // HTTP API Server host
char* mqttServer = "broker.smartliving.io";         

byte mac[] = {  0x90, 0xA2, 0xDA, 0x0D, 0xE1, 0x4B }; // Adapt to your Arduino MAC Address  

int iHumid = 0;
int iTemp = 1;
int iLight = 2;
int iSound = 3;
int iAirQ = 4;
int iGases = 5; //this value is for sensor id, "5" in this case

void callback(char* topic, byte* payload, unsigned int length);
EthernetClient ethClient;
PubSubClient pubSub(mqttServer, 1883, callback, ethClient);

//--------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
{
 
 // pinMode(8, INPUT);
  //wdt_enable(WDTO_8S);
  dht.begin();
  //delay(1000);
}
   if(Device.Connect(mac, httpServer))					          // connect the device with the IOT platform.
  {
    Device.AddAsset(iHumid, "1:Humidity", "Humidity Sensor",false, "int");
    Device.AddAsset(iTemp, "2:Temperature", "Temperature Sensor",false, "int");
    Device.AddAsset(iLight, "3:Light", "Light Sensor",false, "int");
    Device.AddAsset(iSound, "4:Sound", "Sound Sensor",false, "int");
    Device.AddAsset(iAirQ, "5:AirQ", "Air Quality Sensor",false, "int");
    Device.AddAsset(iGases, "6:Gas", "Gas Sensor",false, "int");
    Device.Subscribe(pubSub);						        // make certain that we can receive message from the iot platform (activate mqtt)
  }
  else 
    while(true);                                                                //can't set up the device on the cloud, can't continue, so put the app in an ethernal loop so it doesn't do anything else anymore.								
}
unsigned long time;							        //only send every x amount of time.
//unsigned int prevVal =0;

//-----------------------------------------------------------------------------
void loop() {
 
  unsigned long curTime = millis();
  if (curTime > (time + 10000)) 							// publish light reading every 5 seconds to sensor 1
  {
  //float 
  int h = dht.readHumidity();
  Device.Send(String(h), 0);
  
  //float 
  int t = dht.readTemperature();
  Device.Send(String(t), 1);
  
  int l = analogRead(light_sensor_pin);
  Device.Send(String(l), 2);
  
  int s = analogRead(sound_sensor_pin);
  Device.Send(String(s), 3);
  
  int a = analogRead(airquality_sensor_pin);
  Device.Send(String(a), 4);
  
  int g = analogRead(gas_sensor_pin);
  Device.Send(String(g), 5); //(String(sensor value), sensorid)
  time = curTime;
  }
  Device.Process(); 
}

// Callback function: handles messages that were sent from the iot platform to this device.
void callback(char* topic, byte* payload, unsigned int length) 
{ 
  String msgString; 
  {                                                            //put this in a sub block, so any unused memory can be freed as soon as possible, required to save mem while sending data
	char message_buff[length + 1];	                      //need to copy over the payload so that we can add a /0 terminator, this can then be wrapped inside a string for easy manipulation.
	strncpy(message_buff, (char*)payload, length);        //copy over the data
	message_buff[length] = '\0';		              //make certain that it ends with a null			
		  
	msgString = String(message_buff);
	msgString.toLowerCase();			      //to make certain that our comparison later on works ok (it could be that a 'True' or 'False' was sent)
  }
  char* idOut = NULL;
  {	                                                      //put this in a sub block, so any unused memory can be freed as soon as possible, required to save mem while sending data
	int topicLength = strlen(topic);
	
	Serial.print("Payload: ");                            //show some debugging.
	Serial.println(msgString);
	Serial.print("topic: ");
	Serial.println(topic);
	
  }
  if(idOut != NULL)                //also let the iot platform know that the operation was succesful: give it some feedback. This also allows the iot to update the GUI's correctly & run scenarios.
    Device.Send(msgString, *idOut);    
}
