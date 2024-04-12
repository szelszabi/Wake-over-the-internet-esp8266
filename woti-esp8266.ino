#include "ESP8266WiFi.h"
#include <WiFiUdp.h>
#include <WakeOnLan.h>
#include "ESP8266HTTPClient.h"
#include <ESP8266WebServer.h>
WiFiUDP UDP;
WakeOnLan WOL(UDP);  // Pass WiFiUDP class
// Replace with your network credentials

const char* http_usr PROGMEM = ""; // HTTP basicauth username
const char* http_psw PROGMEM = "";        // HTTP basicauth password
const char* ssid PROGMEM = "";      // Wi-Fi SSID
const char* password PROGMEM = ""; // If your WiFi doesn't have password change line 27's password to 'NULL'
const char* mac PROGMEM = ""; // You can request this by typing 'ipconfig /all' in a windows cmd, 'ifconfig -a' if you're on linux.
const char page[] PROGMEM = "<!DOCTYPE html><html>"\
  "<head><meta charset=\"UTF-8\" name=\"viewport\" content=\"width=device-width, initial-scale=1\">" \
  "<link rel=\"icon\" href=\"data:,\">" \
  "<title>Wake-over-the-internet</title>" \
  "<style>input[type=submit], button{" \
  "background-color: #fade64; width: 100%; height: 760px; border: none; color: #edc821;"\
  "font-family:\"Open Sans\", \"Comic Mono\", \"Comic Sans MS\", \"Comic Sans\", \"Suplexmentary Comic NC\", \"Bubblegum Sans\";"\
  "font-size: 500%; font-stretch: expanded;}</style></head>" \
  " <body><a href=\"/start\"><button><h1>Start Me!</h1></button></a></body>"\
  "<script>let timeout;let url = document.URL.toString();if (url.includes(\"start\")) {timer();}" \
  "function timer(){timeout = setTimeout(onClick(), 500)}" \
  " function onClick(){console.log(\"hello\");window.location.href = \"./\"}</script></html>";
// Set web server port number to 80
ESP8266WebServer server(80); // If you want this to work everywhere, you need to port-forward this port.

void handleRoot() 
{
  server.send(200, F("text/html"), page);
}

void handleStart() 
{
  WOL.sendMagicPacket(mac);
  handleRoot();
}

void handleNotFound()
{
  server.send(404, F("text/plain"), F("404: Not found"));
}

void setup() {
  Serial.begin(115200);
  pinMode(D2, OUTPUT);
  digitalWrite(D2, LOW);
  // Connect to Wi-Fi network with SSID and password
  Serial.println(F(""));
  Serial.println(F(""));
  Serial.println(F("Connecting to: "));
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  
  WOL.calculateBroadcastAddress(WiFi.localIP(), WiFi.subnetMask());
  Serial.println(F(""));
  Serial.println(F("WiFi Connected Successfully:"));
  Serial.println(F("IP address: "));
  Serial.print(WiFi.localIP());
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  server.begin();
  Serial.println(WiFi.macAddress()); // Prints out your MAC Address so that you can put a static lease on it, so later you can port-forward that IP address.
  server.on(F("/"), []() {
    if (!server.authenticate(http_usr, http_psw)) {
      return server.requestAuthentication();
    }
    handleRoot();
    });
  server.on(F("/start"), [](){
    if (!server.authenticate(http_usr, http_psw)) {
      return server.requestAuthentication();
    }
    handleStart();
    });
  server.onNotFound(handleNotFound);
}
void loop() {
  server.handleClient();
}