#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <DNSServer.h>
#include <WiFiManager.h>  
#include <ThingsBoard.h>


//#define WIFI_AP "Nama SSID"
//#define WIFI_PASSWORD "Paswword SSID"

#define TOKEN "GR_HOUSE_HYD_S0001"
char thingsboardServer[] = "iotplatform.soltekno.com";

WiFiManager wifiManager;
WiFiClient wifiClient;
ThingsBoard tb(wifiClient);
int status = WL_IDLE_STATUS;

float temperature, tinggiAir, pH, TDS;


//Location data
float latitude = -6.253688;
float longitude = 106.977677;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(57600);
  InitWiFi();
  if ( !tb.connected() ) {
    reconnect();
    }
}

void InitWiFi()
{
  Serial.println("Connecting to AP ...");
  // attempt to connect to WiFi network
  wifiManager.autoConnect();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to Wifi AP");
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WIFI_AP);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

String splitString(String data, char separator, int index){
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}


void reconnect(){
  // Loop until we're reconnected
  while (!tb.connected()) {
    status = WiFi.status();
    if ( status != WL_CONNECTED) {
          wifiManager.autoConnect();
          Serial.println("connected...yeey :)");
    }
    Serial.print("Connecting to ThingsBoard node ...");
    if ( tb.connect(thingsboardServer, TOKEN) ) {
      Serial.println( "[DONE]" );
    } else {
      Serial.print( "[FAILED]" );
      Serial.println( " : retrying in 2 seconds]" );
      // Wait 1 seconds before retrying
      delay( 1000 );
    }
  }
}


void loop() {  
  delay(2000);

  if(Serial.available()){
    String msg = "";    
    while(Serial.available()){
      msg += char(Serial.read());
      delay(50);
    }   

    if ( !tb.connected() ) {
    reconnect();
    }

  // Send device static location data
    tb.sendTelemetryFloat("latitude", latitude);
    tb.sendTelemetryFloat("longitude", longitude);    
    
    temperature = splitString(msg, ';', 0).toFloat();
    tinggiAir = splitString(msg, ';', 1).toFloat();
    pH = splitString(msg, ';', 2).toFloat();
    TDS = splitString(msg, ';', 3).toFloat();

  // Sending data to thingsboard
    tb.sendTelemetryFloat("temperature", temperature);
    tb.sendTelemetryFloat("Batas Air", tinggiAir);
    
    tb.sendTelemetryFloat("pH", pH);
    tb.sendTelemetryFloat("TDS", TDS);

  Serial.print(msg);
  }    
  
  tb.loop();  
}
