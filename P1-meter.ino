#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <SoftwareSerial.h>

const char* ssid = "<local ssid>";
const char* password = "<wifi password>";
const char* host = "p1meter"; // hostname of this "server"
const int requestPin =  D5; // Dataport for reading dmsr

ESP8266WebServer server(9100); // Webserver port, Prometheus node eporter uses 9100 by default

const char* cmd = "Empty";
String test = "";  

const int led = 13;
char input; // incoming serial data (byte)
bool readnextLine = false;

#define BUFSIZE 100
char buffer[BUFSIZE]; //Buffer for serial data to find \n .
int bufpos    = 0;
long mEVLT    = 0; //1-0:1.8.1 Meter reading Electrics - consumption low tariff
long mEVHT    = 0; //1-0:1.8.2 Meter reading Electrics - consumption high tariff
long mEVLTw   = 0; //1-0:1.8.1 Meter reading Electrics - consumption low tariff Watts
long mEVHTw   = 0; //1-0:1.8.2 Meter reading Electrics - consumption high tariff Watts
long mEOLT    = 0; //1-0:2.8.1 Meter reading Electrics - return low tariff
long mEOHT    = 0; //1-0:2.8.2 Meter reading Electrics - return high tariff
long mEAL     = 0; //1-0:1.7.0 Meter reading Electrics - Actual consumption low tariff
long mEAH     = 0; //1-0:2.7.0 Meter reading Electrics - Actual consumption high tariff
long mEPF     = 0; //0-0:96.7.21 Power faillures any phase
long mELPF    = 0; //0-0:96.7.9  Long Power faillures any phase
long mEVS1    = 0; //1-0:32.32.0 Voltage sags L1
long mEVS2    = 0; //1-0:52.32.0 Voltage sags L2
long mEVS3    = 0; //1-0:72.32.0 Voltage sags L3
long mEVR1    = 0; //1-0:32.36.0 Voltage rise L1
long mEVR2    = 0; //1-0:52.36.0 Voltage rise L2
long mEVR3    = 0; //1-0:72.36.0 Voltage rise L3
long mEIC1    = 0; //1-0:31.7.0 Instantaneous current L1
long mEIC2    = 0; //1-0:51.7.0 Instantaneous current L2
long mEIC3    = 0; //1-0:71.7.0 Instantaneous current L3
long mEIP1    = 0; //1-0:21.7 Instantaneous active Power L1
long mEIP2    = 0; //1-0:41.7 Instantaneous active Power L2
long mEIP3    = 0; //1-0:61.7 Instantaneous active Power L3
long mG       = 0; //0-1:24.2.1 Meter reading Gas

long cEVLT    = 0; 
long cEVHT    = 0; 
long cEVLTw   = 0; 
long cEVHTw   = 0; 
long cEOLT    = 0; 
long cEOHT    = 0; 
long cEAL     = 0; 
long cEAH     = 0; 
long cEPF     = 0; 
long cELPF    = 0; 
long cEVS1    = 0; 
long cEVS2    = 0; 
long cEVS3    = 0; 
long cEVR1    = 0; 
long cEVR2    = 0; 
long cEVR3    = 0; 
long cG       = 0;

long sEVLT    = 0; 
long sEVHT    = 0; 
long sEVLTw   = 0; 
long sEVHTw   = 0; 
long sEOLT    = 0; 
long sEOHT    = 0; 
long sEAL     = 0; 
long sEAH     = 0; 
long sEPF     = 0; 
long sELPF    = 0; 
long sEVS1    = 0; 
long sEVS2    = 0; 
long sEVS3    = 0; 
long sEVR1    = 0; 
long sEVR2    = 0; 
long sEVR3    = 0; 
long sG       = 0;

void handleRoot() {
  digitalWrite(led, 1);
  server.send(200, "text/plain", "This is the DSMR root, visit /metrics for output");
  digitalWrite(led, 0);
}

void handleNotFound(){
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void decodeTelegram() {
  long tl = 0;
  long tld = 0;
  long test = 0;
  
  if (Serial.available()) {
    input = Serial.read();
    Serial.print(input);
    char inChar = (char)input;
    // Fill buffer up to and including a new line (\n)
    buffer[bufpos] = input&127;
    bufpos++;

    if (input == '\n') { // We received a new line (data up to \n)
      if (sscanf(buffer,"1-0:1.8.1(%ld.%ld" ,&tl, &tld)==2){
        if (tl > mEVLT){
          mEVLT = tl;
        }
      }

      if (sscanf(buffer,"1-0:1.8.2(%ld.%ld" ,&tl, &tld)==2){
        if (tl > mEVHT){
          mEVHT = tl;
        }
      }

      if (sscanf(buffer,"1-0:1.8.1(%ld.%ld" ,&tl, &tld)==2){
        test = (tl*1000)+tld;
        if (test > mEVLTw){
          mEVLTw = test;
        }
      }

      if (sscanf(buffer,"1-0:1.8.2(%ld.%ld" ,&tl, &tld)==2){
        test = (tl*1000)+tld;
        if (test > mEVHTw){
          mEVHTw = test;
        }
      }

      if (sscanf(buffer,"1-0:2.8.1(%ld.%ld" ,&tl, &tld)==2){
        if (tl > mEOLT){
          mEOLT = tl;
        }
      }

      if (sscanf(buffer,"1-0:2.8.2(%ld.%ld" ,&tl, &tld)==2){
        if (tl > mEOHT){
          mEOHT = tl;
        }
      }

      if (sscanf(buffer,"1-0:1.7.0(%ld.%ld" ,&tl , &tld) == 2){
        if (tl > mEAL){
          mEAL = tl;
        }
      }

      if (sscanf(buffer,"1-0:2.7.0(%ld.%ld" ,&tl , &tld) == 2){
        if (tl > mEAH){
          mEAH = tl;
        }
      }

      if (sscanf(buffer,"0-0:96.7.21(%ld.%ld" ,&tl , &tld) == 2){
          mEPF = tl;
      }

      if (sscanf(buffer,"0-0:96.7.9(%ld.%ld" ,&tl , &tld) == 2){
          mELPF = tl;
      }

      if (sscanf(buffer,"1-0:32.32.0(%ld" ,&tl) == 1){
         mEVS1 = tl;
      }

      if (sscanf(buffer,"1-0:52.32.0(%ld" ,&tl) == 1){
         mEVS2 = tl;
      }

      if (sscanf(buffer,"1-0:72.32.0(%ld" ,&tl) == 1){
         mEVS3 = tl;
      }

      if (sscanf(buffer,"1-0:32.36.0(%ld" ,&tl) == 1){
         mEVR1 = tl;
      }

      if (sscanf(buffer,"1-0:52.36.0(%ld" ,&tl) == 1){
         mEVR2 = tl;
      }

      if (sscanf(buffer,"1-0:72.36.0(%ld" ,&tl) == 1){
         mEVR3 = tl;
      }

      if (sscanf(buffer,"1-0:31.7.0(%ld" ,&tl) == 1){
         mEIC1 = tl;
      }

      if (sscanf(buffer,"1-0:51.7.0(%ld" ,&tl) == 1){
         mEIC2 = tl;
      }

      if (sscanf(buffer,"1-0:71.7.0(%ld" ,&tl) == 1){
         mEIC3 = tl;
      }

      if (sscanf(buffer,"1-0:21.7.0(%ld.%ld" ,&tl , &tld) == 2){
         mEIP1 = (tl*1000)+tld;
      }

      if (sscanf(buffer,"1-0:41.7.0(%ld.%ld" ,&tl , &tld) == 2){
         mEIP2 = (tl*1000)+tld;
      }

      if (sscanf(buffer,"1-0:61.7.0(%ld.%ld" ,&tl , &tld) == 2){
         mEIP3  = (tl*1000)+tld;
      }

      if (strncmp(buffer, "0-1:24.2.1", strlen("0-1:24.2.1")) == 0) {
        if (sscanf(strrchr(buffer, '(') + 1, "%d.%d", &tl, &tld) == 2) {
          test = (tl*1000)+tld;
          if (test > mG){
            mG = test;
          }
        }
      }

      // Empty buffer again (whole array)
      for (int i=0; i<75; i++)
      {
        buffer[i] = 0;
      }
      bufpos = 0;
    }
  }
}

void MetricsRequest() {

 // Calculate change since last check
  cEVLT  = mEVLT - sEVLT;
  cEVHT  = mEVHT - sEVHT;
  cEVLTw = mEVLTw- sEVLTw;
  cEVHTw = mEVHTw- sEVHTw;
  cEOLT  = mEOLT - sEOLT;
  cEOHT  = mEOHT - sEOHT;
  cEAL   = mEAL - sEAL;
  cEAH   = mEAH - sEAH;
  cEPF   = mEPF - sEPF;
  cELPF  = mELPF - sELPF;
  cEVS1  = mEVS1 - sEVS1;
  cEVS2  = mEVS2 - sEVS2;
  cEVS3  = mEVS3 - sEVS3;
  cEVR1  = mEVR1 - sEVR1;
  cEVR2  = mEVR2 - sEVR2;
  cEVR3  = mEVR3 - sEVR3;
  cG     = mG - sG;

 // Build output
  String cmd = "# DSMR Node exporter for prometheus";

  cmd += "\r\n# 1-0:1.8.1 Meter reading Electrics - consumption low tariff in KiloWatt";
  cmd += "\r\ndsmr_electric_volume_low_tarrif_kw{direction=consume,volume=total} ";
  cmd += String(mEVLT);
  cmd += "\r\ndsmr_electric_volume_low_tarrif_kw{direction=consume,volume=change} ";
  cmd += String(cEVLT);
  
  cmd += "\r\n# 1-0:1.8.1 Meter reading Electrics - consumption low tariff in Watt";
  cmd += "\r\ndsmr_electric_volume_low_tarrif_watt{direction=consume,volume=total} ";
  cmd += String(mEVLTw);
  cmd += "\r\ndsmr_electric_volume_low_tarrif_watt{direction=consume,volume=change} ";
  cmd += String(cEVLTw);

  cmd += "\r\n# 1-0:1.8.2 Meter reading Electrics - consumption high tariff in KiloWatt";
  cmd += "\r\ndsmr_electric_volume_high_tarrif_kw{direction=consume,volume=total} ";
  cmd += String(mEVHT);
  cmd += "\r\ndsmr_electric_volume_high_tarrif_kw{direction=consume,volume=change} ";
  cmd += String(cEVHT);

  cmd += "\r\n# 1-0:1.8.2 Meter reading Electrics - consumption high tariff in Watt";
  cmd += "\r\ndsmr_electric_volume_high_tarrif_watt{direction=consume,volume=total} ";
  cmd += String(mEVHTw);
  cmd += "\r\ndsmr_electric_volume_high_tarrif_watt{direction=consume,volume=change} ";
  cmd += String(cEVHTw);

  cmd += "\r\n# 1-0:2.8.1 Meter reading Electrics - return low tariff";
  cmd += "\r\ndsmr_electric_volume_low_tarrif_kw{direction=deliver,volume=total} ";
  cmd += String(mEOLT);
  cmd += "\r\ndsmr_electric_volume_low_tarrif_kw{direction=deliver,volume=change} ";
  cmd += String(cEOLT);

  cmd += "\r\n# 1-0:2.8.2 Meter reading Electrics - return high tariff";
  cmd += "\r\ndsmr_electric_volume_low_tarrif_watt{direction=deliver,volume=total} ";
  cmd += String(mEOHT);
  cmd += "\r\ndsmr_electric_volume_low_tarrif_watt{direction=deliver,volume=change} ";
  cmd += String(cEOHT);

  cmd += "\r\n# 0-0:96.7.21 Power faillures any phase";
  cmd += "\r\ndsmr_electric_power_faillure_short{volume=total} ";
  cmd += String(mEPF);
  cmd += "\r\ndsmr_electric_power_faillure_short{volume=change} ";
  cmd += String(cEPF);

  cmd += "\r\n# 0-0:96.7.9  Long Power faillures any phase";
  cmd += "\r\ndsmr_electric_power_faillure_long{volume=total} ";
  cmd += String(mELPF);
  cmd += "\r\ndsmr_electric_power_faillure_long{volume=change} ";
  cmd += String(cELPF);
  
  cmd += "\r\n# 1-0:32.32.0 Voltage sags L1";
  cmd += "\r\ndsmr_voltage_sags{phase=L1,volume=total} ";
  cmd += String(mEVS1);
  cmd += "\r\ndsmr_voltage_sags{phase=L1,volume=change} ";
  cmd += String(cEVS1);
  
  cmd += "\r\n# 1-0:52.32.0 Voltage sags L2";
  cmd += "\r\ndsmr_voltage_sags{phase=L2,volume=total} ";
  cmd += String(mEVS2);
  cmd += "\r\ndsmr_voltage_sags{phase=L2,volume=change} ";
  cmd += String(cEVS2);
  
  cmd += "\r\n# 1-0:72.32.0 Voltage sags L3";
  cmd += "\r\ndsmr_voltage_sags{phase=L3,volume=total} ";
  cmd += String(mEVS3);
  cmd += "\r\ndsmr_voltage_sags{phase=L3,volume=change} ";
  cmd += String(cEVS3);
  
  cmd += "\r\n# 1-0:32.36.0 Voltage rise L1";
  cmd += "\r\ndsmr_voltage_rise{phase=L1,volume=total} ";
  cmd += String(mEVR1);
  cmd += "\r\ndsmr_voltage_rise{phase=L1,volume=change} ";
  cmd += String(cEVR1);
  
  cmd += "\r\n# 1-0:52.36.0 Voltage rise L2";
  cmd += "\r\ndsmr_voltage_rise{phase=L2,volume=total} ";
  cmd += String(mEVR2);
  cmd += "\r\ndsmr_voltage_rise{phase=L2,volume=change} ";
  cmd += String(cEVR2);
  
  cmd += "\r\n# 1-0:72.36.0 Voltage rise L3";
  cmd += "\r\ndsmr_voltage_rise{phase=L3,volume=total} ";
  cmd += String(mEVR3);
  cmd += "\r\ndsmr_voltage_rise{phase=L3,volume=change} ";
  cmd += String(cEVR3);

  cmd += "\r\n# 1-0:31.7.0 Instantaneous current L1";
  cmd += "\r\ndsmr_instantaneous_current{phase=L1} ";
  cmd += String(mEIC1);

  cmd += "\r\n# 1-0:51.7.0 Instantaneous current L2";
  cmd += "\r\ndsmr_instantaneous_current{phase=L2} ";
  cmd += String(mEIC2);

  cmd += "\r\n# 1-0:71.7.0 Instantaneous current L3";
  cmd += "\r\ndsmr_instantaneous_current{phase=L2} ";
  cmd += String(mEIC3);
  
  cmd += "\r\n# 1-0:21.7 Instantaneous active Power L1";
  cmd += "\r\ndsmr_instantaneous_power{phase=L1} ";
  cmd += String(mEIP1);

  cmd += "\r\n# 1-0:41.7 Instantaneous active Power L2";
  cmd += "\r\ndsmr_instantaneous_power{phase=L2} ";
  cmd += String(mEIP2);

  cmd += "\r\n# 1-0:61.7 Instantaneous active Power L3";
  cmd += "\r\ndsmr_instantaneous_power{phase=L3} ";
  cmd += String(mEIP3);

  cmd += "\r\n# 0-1:24.2.1 Meter reading Gas";
  cmd += "\r\ndsmr_gas{volume=total} ";
  cmd += String(mG);
  cmd += "\r\ndsmr_gas{volume=change} ";
  cmd += String(cG);

  server.send(200, "text/plain", cmd);

 // Store metrics for next check change calculation
  sEVLT  = mEVLT;
  sEVHT  = mEVHT;
  sEVLTw = mEVLTw;
  sEVHTw = mEVHTw;
  sEOLT  = mEOLT;
  sEOHT  = mEOHT;
  sEAL   = mEAL;
  sEAH   = mEAH;
  sEPF   = mEPF;
  sELPF  = mELPF;
  sEVS1  = mEVS1;
  sEVS2  = mEVS2;
  sEVS3  = mEVS3;
  sEVR1  = mEVR1;
  sEVR2  = mEVR2;
  sEVR3  = mEVR3;
  sG = mG;

}

void setup(void){
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for wifi connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

   // Print details to serial console
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin(host)) {
    MDNS.addService("http", "tcp", 80);
    Serial.println("MDNS responder started");
    Serial.print("You can now connect to http://");
    Serial.print(host);
    Serial.println(":9100");
    Serial.print("or http://");
    Serial.print(WiFi.localIP());
    Serial.println(":9100");
  }

  server.on("/", handleRoot);

  server.on("/metrics", [](){
    MetricsRequest();
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  Serial.begin(115200);
  delay(1000);
  
  //Set RTS pin high, so smart meter will start sending telegrams
  pinMode(requestPin, OUTPUT);
  digitalWrite(requestPin, HIGH);
}

void loop(void){
  server.handleClient();
  decodeTelegram();
}

