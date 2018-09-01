//Que hace: 
//envía un  mensajes MQTT por medio de un Arduino UNO y un modulo ESP-01
//elenvia cada segundo, un valor digital y un analogico
//
//utiliza la libreria PubSubClient y WifiESP con el modulo ESP-01 con firmware de comandos AT

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

//definimos los pines para las entradas digitales y analogicas
int pushButton = 4;
int pinAnalogico = 0;

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
  
  //iniciamos el pin donde tenemos el pushbutton//
  pinMode(pushButton, INPUT); 
  
  // Vamos a usar el Serial HW para hacer debugging 
  Serial.begin(9600);
  
  //llamamos a la funcion InitWiFi para conectarnos por WiFi
  InitWiFi();

  //aqui inicializamos nuestro servidor MQTT y el puerto  
  client.setServer( mqttserver, 1883 );

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

  //publica cada 1000ms (1s) un mensaje 
  if ( millis() - lastSend > 1000 ) { // Update and send only after 1 seconds
    publicarDatos();
    lastSend = millis();
  }

  client.loop();
}

void publicarDatos()
{
  Serial.println("Leyendo Datos");

  //tomamos el valor analogico y el estado digital
  int valorAnalogico = analogRead(pinAnalogico);
  int varlorDigital = digitalRead(pushButton);
  
  //mostramos el valor leido  en el serial 
  Serial.print("A0: ");
  Serial.print(valorAnalogico);
  Serial.print("\t");
  Serial.print("D0: ");
  Serial.print(varlorDigital);

  // Enviamos al servidor
  Serial.print( "Enviando por mqtt:" );

  // Prepare a JSON payload string
 // String payload = "{";
 // payload += "\"temperature\":"; payload += temperature; payload += ",";
 // payload += "\"humidity\":"; payload += humidity;
 // payload += "}";

  // Send payload
 // char attributes[100];
 // analogico01.toCharArray( attributes, 100 );
  
  //publica en el topico "arduino/A0", el valor analogico
  char cstr[16];
  itoa(valorAnalogico, cstr, 10);
  
  client.publish( "arduino/A0", cstr );
  Serial.print(valorAnalogico);
  Serial.print("\t"); 
  //publica en el topico "arduino/D0", el valor analogico
// digital01.toCharArray( attributes, 100 );
  itoa(varlorDigital, cstr, 10);
  client.publish( "arduino/D0", cstr );
  Serial.print(varlorDigital);
  Serial.print("\r\n");
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
    } else {
      Serial.print( "[FAILED] [ rc = " );
      Serial.print( client.state() );
      Serial.println( " : retrying in 5 seconds]" );
      // Wait 5 seconds before retrying
      delay( 5000 );
    }
  }
}


