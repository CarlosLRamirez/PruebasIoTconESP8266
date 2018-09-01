/**
 * \file
 *       ESP8266 MQTT Bridge example
 * \author
 *       Tuan PM <tuanpm@live.com>
 */

#include <SoftwareSerial.h>
#include <espduino.h>
#include <mqtt.h>

int led1 = 4;
int led2 = 5;
int led3 = 6;
int led4 = 7;
int button1=8;

///toda esta parte de codigo es por el boton ///////////////////////////////
int state = LOW;      // the current state of the output pin
int reading;           // the current reading from the input pin
int previous = LOW;    // the previous reading from the input pin

// the follow variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long time = 0;         // the last time the output pin was toggled
long debounce = 200;   // the debounce time, increase if the output flickers
///////////////////////////////////////////////////////////////////////////

SoftwareSerial debugPort(2, 3); // RX, TX
ESP esp(&Serial, &debugPort, 4);
MQTT mqtt(&esp);
boolean wifiConnected = false;




void wifiCb(void *response)
{
  uint32_t status;
  RESPONSE res(response);

  if (res.getArgc() == 1)
  {
    res.popArgs((uint8_t *)&status, 4);
    if (status == STATION_GOT_IP)
    {
      debugPort.println("WIFI CONNECTED");
      mqtt.connect("142.93.49.104", 1883, false);
      wifiConnected = true;
      //or mqtt.connect("host", 1883); /*without security ssl*/
    }
    else
    {
      wifiConnected = false;
      mqtt.disconnect();
    }
  }
}

void mqttConnected(void *response)
{
  debugPort.println("Connected");
  mqtt.publish("arduino/led1", "0");
  mqtt.publish("arduino/led2", "0");
  mqtt.publish("arduino/led3", "0");
  mqtt.publish("arduino/led4", "0");

  mqtt.subscribe("arduino/led1"); //or mqtt.subscribe("topic"); /*with qos = 0*/
  mqtt.subscribe("arduino/led2");
  mqtt.subscribe("arduino/led3");
  mqtt.subscribe("arduino/led4");
  mqtt.publish("/topic/0", "data0");
}
void mqttDisconnected(void *response)
{
}
void mqttData(void *response)
{
  RESPONSE res(response);

  debugPort.print("Received: topic=");
  String topic = res.popString();
  debugPort.println(topic);

  debugPort.print("data=");
  String data = res.popString();
  debugPort.println(data);

  if (topic=="arduino/led1"){
    if (data == "1")
    {
      state = HIGH;
      encenderLed(led1);
    }
    if (data == "0")
    {
      state = LOW;
      apagarLed(led1);
     }
  }
  if (topic=="arduino/led2"){
    if (data == "1")
    {
      encenderLed(led2);
    }
    if (data == "0")
    {
      apagarLed(led2);
     }
  }
  if (topic=="arduino/led3"){
    if (data == "1")
    {
      encenderLed(led3);
    }
    if (data == "0")
    {
      apagarLed(led3);
     }
  }
  if (topic=="arduino/led4"){
    if (data == "1")
    {
      encenderLed(led4);
    }
    if (data == "0")
    {
      apagarLed(led4);
     }
  }
  
}
void mqttPublished(void *response)
{
}
void setup()
{


    pinMode(led1,OUTPUT);
  pinMode(led2,OUTPUT);
  pinMode(led3,OUTPUT);
  pinMode(led4,OUTPUT);
  pinMode(button1,INPUT);

  Serial.begin(19200);
  debugPort.begin(19200);

  debugPort.println("poniendo leds en estado inicial apagado");
  apagarLed(led1);
  apagarLed(led2);
  apagarLed(led3);
  apagarLed(led4);
  delay(5000); //quiero ver que realmente se apaguen

 debugPort.println("inizializando cereales");
 
  esp.enable();
  delay(500);
  esp.reset();
  delay(500);
  while (!esp.ready())
    ;

  debugPort.println("ARDUINO: setup mqtt client");
  if (!mqtt.begin("DVES_duino", "admin", "Isb_C4OGD4c3", 120, 1))
  {
    debugPort.println("ARDUINO: fail to setup mqtt");
    while (1)
      ;
  }

  debugPort.println("ARDUINO: setup mqtt lwt");
  mqtt.lwt("/lwt", "offline", 0, 0); //or mqtt.lwt("/lwt", "offline");

  /*setup mqtt events */
  mqtt.connectedCb.attach(&mqttConnected);
  mqtt.disconnectedCb.attach(&mqttDisconnected);
  mqtt.publishedCb.attach(&mqttPublished);
  mqtt.dataCb.attach(&mqttData);

  /*setup wifi*/
  debugPort.println("ARDUINO: setup wifi");
  esp.wifiCb.attach(&wifiCb);

  esp.wifiConnect("SyProTec", "Syprotec1@");

  debugPort.println("ARDUINO: system started");

  /*mi codigo es a partir de aqui*/
  //pin donde vamos a enceder led (salida digital), no puede ser 0,1,2 o 3

  debugPort.println("ya termine el setup");


}

void loop()
{
  esp.process();

  if (wifiConnected)
  {
  }

  reading = digitalRead(button1);

  if (reading == HIGH && previous == LOW && millis() - time > debounce) {
    if (state == HIGH){
      state = LOW;
      debugPort.println("cambio a 0");
      mqtt.publish("arduino/led1", "0");}
         else{
      state = HIGH;
      debugPort.println("cambio a 1");
      mqtt.publish("arduino/led1", "1");}

    time = millis();    
  }
 
  previous = reading;



}

void encenderLed(int LED)
{
  digitalWrite(LED, HIGH);
}

void apagarLed(int LED)
{
  digitalWrite(LED, LOW);
}