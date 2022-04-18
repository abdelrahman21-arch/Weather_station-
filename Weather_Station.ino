#include <WiFi.h>
#include "DHT.h"
#include<PubSubClient.h>
#include<Wire.h>
#include <Adafruit_I2CDevice.h>
#include <Arduino.h>
#include <Adafruit_BMP085.h>
#define I2C_SDA 33
#define I2C_SCL 32 
#define  DHPIN 4
#define DHTTYPE DHT22
#define mqttport 1883
#define channelID 1607902
const char * PROGMEM thingspeak_ca_cert = \
  "-----BEGIN CERTIFICATE-----\n" \
  "MIIDxTCCAq2gAwIBAgIQAqxcJmoLQJuPC3nyrkYldzANBgkqhkiG9w0BAQUFADBs\n" \
  "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
  "d3cuZGlnaWNlcnQuY29tMSswKQYDVQQDEyJEaWdpQ2VydCBIaWdoIEFzc3VyYW5j\n" \
  "ZSBFViBSb290IENBMB4XDTA2MTExMDAwMDAwMFoXDTMxMTExMDAwMDAwMFowbDEL\n" \
  "MAkGA1UEBhMCVVMxFTATBgNVBAoTDERpZ2lDZXJ0IEluYzEZMBcGA1UECxMQd3d3\n" \
  "LmRpZ2ljZXJ0LmNvbTErMCkGA1UEAxMiRGlnaUNlcnQgSGlnaCBBc3N1cmFuY2Ug\n" \
  "RVYgUm9vdCBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMbM5XPm\n" \
  "+9S75S0tMqbf5YE/yc0lSbZxKsPVlDRnogocsF9ppkCxxLeyj9CYpKlBWTrT3JTW\n" \
  "PNt0OKRKzE0lgvdKpVMSOO7zSW1xkX5jtqumX8OkhPhPYlG++MXs2ziS4wblCJEM\n" \
  "xChBVfvLWokVfnHoNb9Ncgk9vjo4UFt3MRuNs8ckRZqnrG0AFFoEt7oT61EKmEFB\n" \
  "Ik5lYYeBQVCmeVyJ3hlKV9Uu5l0cUyx+mM0aBhakaHPQNAQTXKFx01p8VdteZOE3\n" \
  "hzBWBOURtCmAEvF5OYiiAhF8J2a3iLd48soKqDirCmTCv2ZdlYTBoSUeh10aUAsg\n" \
  "EsxBu24LUTi4S8sCAwEAAaNjMGEwDgYDVR0PAQH/BAQDAgGGMA8GA1UdEwEB/wQF\n" \
  "MAMBAf8wHQYDVR0OBBYEFLE+w2kD+L9HAdSYJhoIAu9jZCvDMB8GA1UdIwQYMBaA\n" \
  "FLE+w2kD+L9HAdSYJhoIAu9jZCvDMA0GCSqGSIb3DQEBBQUAA4IBAQAcGgaX3Nec\n" \
  "nzyIZgYIVyHbIUf4KmeqvxgydkAQV8GK83rZEWWONfqe/EW1ntlMMUu4kehDLI6z\n" \
  "eM7b41N5cdblIZQB2lWHmiRk9opmzN6cN82oNLFpmyPInngiK3BD41VHMWEZ71jF\n" \
  "hS9OMPagMRYjyOfiZRYzy78aG6A9+MpeizGLYAiJLQwGXFK3xPkKmNEVX58Svnw2\n" \
  "Yzi9RKR/5CYrCsSXaQ3pjOLAEFe4yHYSkVXySGnYvCoCWw9E1CAx2/S6cCZdkGCe\n" \
  "vEsXCS+0yx5DaMkHJ8HSXPfqIbloEpw8nL+e/IBcm2PN7EeqJSdnoDfzAIJ9VNep\n" \
  "+OkuE6N36B9K\n" \
  "-----END CERTIFICATE-----\n";
TwoWire I2CBMP = TwoWire(0);
Adafruit_BMP085 bmp;
DHT dht(DHPIN,DHTTYPE);
WiFiClient client;
const char mqttUserName[] = "OhosOwItETw0LhcODRU0GAI"; 
const char clientID[] = "OhosOwItETw0LhcODRU0GAI";
const char mqttPass[] = "Eaj3tafNmfRty+ESu7hRmDmL";
const char* ssid = "Your Wifi SSid";
const char* pass = "passsword";
const char* server = "mqtt3.thingspeak.com";
int status = WL_IDLE_STATUS; 
long lastPublishMillis = 0;
int connectionDelay = 1;
int updateInterval = 15;
const int ledPin = 5;
PubSubClient mqttClient( client );
void mqttSubscriptionCallback( char* topic, byte* payload, unsigned int length ) {
  // Print the details of the message that was received to the serial monitor.
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}
void mqttSubscribe( long subChannelID ){
  String myTopic = "channels/"+String( subChannelID )+"/subscribe";
  mqttClient.subscribe(myTopic.c_str());
}


void Init_Wifi()
{
WiFi.mode(WIFI_STA);
WiFi.begin(ssid,pass);
Serial.println("Connecting...");
while(WiFi.status()!=WL_CONNECTED)
{
  Serial.println('.');
  delay(1000);
}
Serial.println(WiFi.localIP());
Serial.println("\n connected!");
}
void mqttPublish(long pubChannelID, String message) {
  String topicString ="channels/" + String( pubChannelID ) + "/publish";
  mqttClient.publish( topicString.c_str(), message.c_str() );
}
void mqttConnect()
{
  while(!mqttClient.connected() )
  
  {
  if(mqttClient.connect(clientID,mqttUserName,mqttPass ) ) 
  {
    Serial.print( "MQTT to " );
      Serial.print( server );
      Serial.print (" at port ");
      Serial.print( mqttport );
      Serial.println( " successful." );
    
    
  }
else
{
  Serial.print( "MQTT connection failed, rc = " );
      // See https://pubsubclient.knolleary.net/api.html#state for the failure code explanation.
      Serial.print( mqttClient.state() );
      Serial.println( " Will try again in a few seconds" );
      delay( connectionDelay*1000 );

}
    
    
  }
}

void init_Bmp ()
{
  I2CBMP.begin(I2C_SDA,I2C_SCL,40000);
  bool status=bmp.begin(0x74,&I2CBMP);
  if (!status)
  {
    Serial.println("BMP180 Sensor not found ! ! !");
    while (1)
    {
    
    }
  }
  Serial.println("--Default Test--");
  delay(1000);
  Serial.println();
}





void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode (ledPin, OUTPUT);
  delay(1000);
  Init_Wifi();
  init_Bmp();
  dht.begin();
  mqttClient.setServer(server,mqttport);
  mqttClient.setCallback(mqttSubscriptionCallback);
  mqttClient.setBufferSize(2048);
  
  }

void loop() {
  if(WiFi.status()!=WL_CONNECTED)
  {
    Init_Wifi();
  }
  if(!mqttClient.connected())
  {
     mqttConnect(); 
     mqttSubscribe( channelID );
  }
  mqttClient.loop();
  
  delay(2000);
  float h=dht.readHumidity();
  // default temp  in celsius
  float t=dht.readTemperature();
  int p = bmp.readPressure();
  //float p_atm=p*0.00000986923;
  if(p<=101870)
  {
 digitalWrite (ledPin, HIGH);
 delay(500);
 digitalWrite(ledPin,LOW);
 delay(500);
    
  }
  
  if ( abs(millis() - lastPublishMillis) > updateInterval*1000) {
    mqttPublish( channelID, (String("&field1="+String(h)+"&field2="+String(t)+"&field3="+String(p))));
  //  mqttPublish( channelID, (String("field2=")+String(t) ));
    //mqttPublish( channelID, (String("field3=")+String(p) ));
    
    lastPublishMillis = millis();
  }
  
if(isnan(h)||isnan(t))
{
  Serial.println("Failed to read a correct number from sensor");
  return;
} 

}
