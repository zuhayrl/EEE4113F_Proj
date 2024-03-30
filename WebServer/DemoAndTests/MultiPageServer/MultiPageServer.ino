/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com  
*********/

// Import required libraries
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
//#include "SPIFFS.h"
#include "index.h"
#include "led.h"

#define LED_PIN 12  // ESP32 pin GPIO18 connected to LED

// Replace with your network credentials
const char* ssid = "ESP_Hotspot";
const char* password = NULL;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

//Set LED
int LED_state = LOW;

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  // Initialize SPIFFS
  //if(!SPIFFS.begin(true)){
  //  Serial.println("An Error has occurred while mounting SPIFFS");
  //  return;
  //}

  // Connect to Wi-Fi
  //WiFi.begin(ssid, password);
  //while (WiFi.status() != WL_CONNECTED) {
  //  delay(1000);
  //  Serial.println("Connecting to WiFi..");
  //}

  //Create Acess Point
  WiFi.softAP(ssid, password);

  // Print ESP32 Local IP Address
  //Serial.println(WiFi.localIP());
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);



  //WEBPAGE STUFF
  //Index Page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("Web Server: home page");
    String html = HTML_CONTENT_HOME;  // Use the HTML content from the index.h file
    request->send(200, "text/html", html);
  });

  server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("Web Server: home page");
    String html = HTML_CONTENT_HOME;  // Use the HTML content from the index.h file
    request->send(200, "text/html", html);
  });

  // LED Page
  server.on("/led.html", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.print("Web Server: LED page");
    
    // Check for the 'state' parameter in the query string
    if (request->hasArg("state")) {
      String state = request->arg("state");
      if (state == "on") {
        LED_state = HIGH;
      } else if (state == "off") {
        LED_state = LOW;
      }

      // control LED here
      digitalWrite(LED_PIN, LED_state);
      Serial.print(" => turning LED to ");
      Serial.print(state);
    }
    Serial.println();

    String html = HTML_CONTENT_LED;                         // Use the HTML content from the led.h file
    html.replace("%STATE%", LED_state ? "ON" : "OFF");  // update the LED state
    request->send(200, "text/html", html);
  });  




  // Start server
  server.begin();
  Serial.println("ESP32 Web server started");
}
 
void loop(){
  
}
