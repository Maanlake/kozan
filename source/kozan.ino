/*--------------------------------------------------
HTTP 1.1 Webserver as AccessPoint for ESP8266 
for ESP8266 adapted Arduino IDE

by Stefan Thesen 08/2015 - free for anyone

Does HTTP 1.1 with defined connection closing.
Handles empty requests in a defined manner.
Handle requests for non-exisiting pages correctly.

This demo allows to switch two functions:
Function 1 creates serial output and toggels GPIO2
Function 2 just creates serial output.

Serial output can e.g. be used to steer an attached
Arduino, Raspberry etc.
--------------------------------------------------*/

#include <FS.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>

const char* ssid = "DONOTCONNECTME";
const char* password = "";  // set to "" for open access point w/o passwortd

unsigned long ulReqcount;


// Create an instance of the server on Port 80
ESP8266WebServer server(80);
IPAddress ip(192, 168, 0, 1);
IPAddress gateway(192,168,0,1);
IPAddress dns(192,168,0,1);
IPAddress subnet(255,255,255,0);

// DNS server
const byte DNS_PORT = 53;
DNSServer dnsServer;

void httpDefault()
{
  Serial.println("redirect->re.bel");
  server.sendHeader("Location", "http://re.bel/", true);
  server.send(302, "text/plain", "");
  server.client().stop();
}

void httpHome()
{
  if (server.hostHeader() != String("re.bel")) {
    Serial.println("server.hostHeader()");
    return httpDefault();
  }
  Serial.println("HOME");
  bool ok = SPIFFS.begin();
  if (ok) {
    Serial.println("ok");
    bool exist = SPIFFS.exists("/index.html.gz");

    if (exist) {
      Serial.println("The file exists!");

      File f = SPIFFS.open("/index.html.gz", "r");
      if (!f) {
        Serial.println("Some thing went wrong trying to open the file...");
      }
      else {
        int s = f.size();
        Serial.printf("Size=%d\r\n", s);
        String data = f.readString();
        Serial.println(data);

        f.close();
      }
    }
    else {
      Serial.println("No such file found.");
    }
  }
  File file = SPIFFS.open("/index.html.gz", "r");
  server.streamFile(file, "text/html");
  file.close();
}
void httpSVG()
{
  Serial.println("SVG");
  File file = SPIFFS.open("/cyclops.svg", "r");
  server.streamFile(file, "image/svg+xml");
  file.close();
}
void setup() 
{
   /* Setup the DNS server redirecting all the domains to the IP */  
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", ip);
  Serial.begin(115200);
  
  // AP mode
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(ip,gateway, subnet);
  server.on("/", httpHome);
  server.on("/cyclops.svg", httpSVG);
  server.onNotFound(httpDefault);
  server.begin();
}

void loop() 
{ 
  dnsServer.processNextRequest();
  server.handleClient();
}

