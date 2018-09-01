//Prueba de Subscripcion a un MQTT con un  un Arduino UNO y un modulo ESP-01
//el programa se subscribe a un topico, y esta esuchando mediante un callback
//al recibir algo lo envia por el puerto serial

//El programa funciona de forma erratica, y los mensaje no siempre se reciben
//en internet se reportan muchos problemas similares, usando la libreria PubSubClient (Nick O'Leary)
//con el ESP8266 conectado a un Arduino

//https://github.com/knolleary/pubsubclient/issues/163
//https://github.com/knolleary/pubsubclient/issues/284
//https://forum.arduino.cc/index.php?topic=451326.0
//https://sonyarouje.com/2016/03/15/mqtt-communication-with-arduino-using-esp8266-esp-01/
//https://github.com/knolleary/pubsubclient/issues/236

//Al parecer el problema esta en la rutina client.loop, la cual para que el callback funcione
//debe ser llamada  frecuentemente, en el ejemplo de la libreria se llama en cada void loop;
//sin embargo esto genera muchos datos en el puerto serial y hace que se pierda el mensaje.

//agregandole algun timer para que el client.loop se ejecute cada 100ms, ayuda y empiezan a aparecer
//algunos mensajes (tal vez un 50%), pero sigue siendo erratico.

//una posible solucion podria ser utilizar el puerto Serial de HW para conectar el ESP01, y utilizar
//una velocidad mas alta, y hacer debug por un puerto Serial por SW, tambien hay quienes sugieren meterse
//en la libreria y cambiar algunas cosas, o buscar otra librera PubSub 

//una mejor solucion seria programar directament en el ESP-01 y que este al tener los mensajes ya capturados
//los envie al Ardunio

#include <WiFiEspClient.h>
#include <WiFiEsp.h>
#include <WiFiEspUdp.h>
#include <PubSubClient.h>
#include "SoftwareSerial.h"

//#define WIFI_AP "FabLabTIC's"
//#define WIFI_PASSWORD "Intec@p2018*"

//credenciales de la red Wifi//
#define WIFI_AP "SyProTec"
#define WIFI_PASSWORD "Syprotec1@"

//direccion del servidor mqtt (mqtt broker)//
char mqttserver[] = "142.93.49.104";


// Inicializar el objeto Ethernet client (ESP8266)
WiFiEspClient espClient;

//aqui decimos que vamos a usar el ESP8266 como cliente MQTT//
PubSubClient client(espClient);

//definimos un puerto serial por software en el Arduino, para poder comunicarnos con el ESP8266//
//el ESP8266 debe estar en 9600, para que el serial software no tenga problemas//
SoftwareSerial soft(2, 3); // RX, TX

//estado incial del WiFi
int status = WL_IDLE_STATUS;

//variable para llevar el timer
unsigned long lastSend;

void setup() {
  
  // Vamos a usar el Serial HW para hacer debugging 
  Serial.begin(9600);
  
  //llamamos a la funcion InitWiFi para conectarnos por WiFi
  InitWiFi();

  //aqui inicializamos nuestro servidor MQTT y el puerto  
  client.setServer( mqttserver, 1883 );

  //establecemos el callback para que este "capture" los mensajes de un topico dado
  client.setCallback(callback);

  //iniciamos la variable
  lastSend = 0;
}

void loop() {


  //verifica si el WiFi sigue conectado, si no intenta hasta que se logre conectar
  status = WiFi.status();
  if ( status != WL_CONNECTED) {
    while ( status != WL_CONNECTED) {
      Serial.print("Attempting to connect to WPA SSID: ");
      Serial.println(WIFI_AP);
      // Connect to WPA/WPA2 network
      status = WiFi.begin(WIFI_AP, WIFI_PASSWORD);
      delay(500);
    }
    Serial.println("Connected to AP");
  }

  if ( !client.connected() ) {
    reconnect();
  }

  //agrego un timer para ejecutar el client.loop cada cierto tiempo, y envio un mensaje debug  
  if ( millis() - lastSend > 200 ) { // Update and send only after 1 seconds
    Serial.println("bip");
    client.loop();
    
    lastSend = millis();
    
  }
   
 
 
 
}

//print any message received for subscribed topic
void callback(char* topic, byte* payload, unsigned int length) {
Serial.print("Message arrived [");
Serial.print(topic);

Serial.print("] ");
for (int i=0;i<length;i++) {
char receivedChar = (char)payload;
Serial.print(receivedChar);
if (receivedChar == '0')
Serial.println("Off");
if (receivedChar == '1')
Serial.println("On");

}
Serial.println();
}


void InitWiFi()
{
  // initialize serial for ESP module
  soft.begin(9600);
  // initialize ESP module
  WiFi.init(&soft);
  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  Serial.println("Connecting to AP ...");
  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(WIFI_AP);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(WIFI_AP, WIFI_PASSWORD);
    delay(500);
  }
  Serial.println("Connected to AP");
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Conectadores al servidor MQTT ...");
    // Attempt to connect (clientId, username, password)
    if ( client.connect("ArduinoClient", NULL, NULL) ) {
      Serial.println( "[DONE]" );
      client.publish("arduino/A0","hello world");
      //esta linea es donde me subscribo al topico
      client.subscribe("arduino/D0",0);
    } else {
      Serial.print( "[FAILED] [ rc = " );
      Serial.print( client.state() );
      Serial.println( " : retrying in 5 seconds]" );
      // Wait 5 seconds before retrying
      delay( 5000 );
    }
  }
}


