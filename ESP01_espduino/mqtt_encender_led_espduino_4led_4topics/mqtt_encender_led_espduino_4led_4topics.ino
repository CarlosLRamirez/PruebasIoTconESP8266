/**
 * \file
 *       ESP8266 MQTT Bridge example
 * \author
 *       Tuan PM <tuanpm@live.com>
 */

//programa que recibe mensajes MQTT para  controlar el estado de 4 leds
//tambien lee 4 pushbuttons y envia su estado por MQTT
//utiliza la libreria espduino, la cual funciona mejor que la PUBSUB con el ESP-01 conectado
//a un Ardunio, pero para usar esta libreria hay que cargar un firmware diferente en el ESP-01,
//(no es el firmware de comandos AT).
//funciona muy bien la subscripcion y publicacion de mensajes MQTT


#include <SoftwareSerial.h>
#include <espduino.h>
#include <mqtt.h>

int led1 = 4;
int led2 = 5;
int led3 = 6;
int led4 = 7;
int button1 = 8;
int button2 = 9;
int button3 = 10;
int button4 = 11;

///toda esta parte de codigo es por el boton ///////////////////////////////
int state1 = LOW; // the current state of the output pin
int state2 = LOW; // the current state of the output pin
int state3 = LOW; // the current state of the output pin
int state4 = LOW; // the current state of the output pin

int reading1; // the current reading from the input pin
int reading2; // the current reading from the input pin
int reading3; // the current reading from the input pin
int reading4; // the current reading from the input pin

int previous1 = LOW; // the previous reading from the input pin
int previous2 = LOW; // the previous reading from the input pin
int previous3 = LOW; // the previous reading from the input pin
int previous4 = LOW; // the previous reading from the input pin

// the follow variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long time = 0;       // the last time the output pin was toggled
long debounce = 200; // the debounce time, increase if the output flickers
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

    //al encender el arduino, inicializamos todas los topicos en 0
    mqtt.publish("arduino/led1", "0");
    mqtt.publish("arduino/led2", "0");
    mqtt.publish("arduino/led3", "0");
    mqtt.publish("arduino/led4", "0");

    mqtt.subscribe("arduino/led1"); //or mqtt.subscribe("topic"); /*with qos = 0*/
    mqtt.subscribe("arduino/led2");
    mqtt.subscribe("arduino/led3");
    mqtt.subscribe("arduino/led4");

    //mensaje de control
    mqtt.publish("arduino/control", "data0");
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

    if (topic == "arduino/led1")
    {
        if (data == "1")
        {
            state1 = HIGH;
            encenderLed(led1);
        }
        if (data == "0")
        {
            state1 = LOW;
            apagarLed(led1);
        }
    }
    if (topic == "arduino/led2")
    {
        if (data == "1")
        {
            state2 = HIGH;
            encenderLed(led2);
        }
        if (data == "0")
        {
            state2 = LOW;
            apagarLed(led2);
        }
    }
    if (topic == "arduino/led3")
    {
        if (data == "1")
        {
            state3 = HIGH;
            encenderLed(led3);
        }
        if (data == "0")
        {
            state3 = LOW;
            apagarLed(led3);
        }
    }
    if (topic == "arduino/led4")
    {
        if (data == "1")
        {
            state4 = HIGH;
            encenderLed(led4);
        }
        if (data == "0")
        {
            state3 = LOW;
            apagarLed(led4);
        }
    }
}
void mqttPublished(void *response)
{
}
void setup()
{

    pinMode(led1, OUTPUT);
    pinMode(led2, OUTPUT);
    pinMode(led3, OUTPUT);
    pinMode(led4, OUTPUT);
    pinMode(button1, INPUT);
    pinMode(button2, INPUT);
    pinMode(button3, INPUT);
    pinMode(button4, INPUT);

    Serial.begin(19200);
    debugPort.begin(19200);

    debugPort.println("poniendo leds en estado inicial apagado");
    apagarLed(led1);
    apagarLed(led2);
    apagarLed(led3);
    apagarLed(led4);


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
}

void loop()
{
    esp.process();

    if (wifiConnected)
    {
    }

    reading1 = digitalRead(button1);
    reading2 = digitalRead(button2);
    reading3 = digitalRead(button3);
    reading4 = digitalRead(button4);
    

    if (reading1 == HIGH && previous1 == LOW && millis() - time > debounce)
    {
        if (state1 == HIGH)
        {
            state1 = LOW;
            debugPort.println("cambio button1 a 0");
            mqtt.publish("arduino/led1", "0");
        }
        else
        {
            state1 = HIGH;//
            debugPort.println("cambio button1 a 1");
            mqtt.publish("arduino/led1", "1");
        }

        time = millis();
    }

    
    if (reading2 == HIGH && previous2 == LOW && millis() - time > debounce)
    {
        if (state2 == HIGH)
        {
            state2 = LOW;
            debugPort.println("cambio button2 a 0");
            mqtt.publish("arduino/led2", "0");
        }
        else
        {
            state2 = HIGH;//
            debugPort.println("cambio button2 a 1");
            mqtt.publish("arduino/led2", "1");
        }

        time = millis();
    }

    
    if (reading3 == HIGH && previous3 == LOW && millis() - time > debounce)
    {
        if (state3 == HIGH)
        {
            state3 = LOW;
            debugPort.println("cambio button3 a 0");
            mqtt.publish("arduino/led3", "0");
        }
        else
        {
            state3 = HIGH;//
            debugPort.println("cambio button3 a 1");
            mqtt.publish("arduino/led3", "1");
        }

        time = millis();
    }

    
    if (reading4 == HIGH && previous4 == LOW && millis() - time > debounce)
    {
        if (state4 == HIGH)
        {
            state4 = LOW;
            debugPort.println("cambio button4 a 0");
            mqtt.publish("arduino/led4", "0");
        }
        else
        {
            state4 = HIGH;//
            debugPort.println("cambio button4 a 1");
            mqtt.publish("arduino/led4", "1");
        }

        time = millis();
    }

    
    

    previous1 = reading1;
    previous2 = reading2;
    previous3 = reading3;
    previous4 = reading4;
    
}

void encenderLed(int LED)
{
    digitalWrite(LED, HIGH);
}

void apagarLed(int LED)
{
    digitalWrite(LED, LOW);
}