#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>

//const char* ssid = "alkh6397";
//const char* password = "cqtw5680";
const char* ssid = "yourssid";
const char* password = "yourpasswd";

// Your Domain name with URL path or IP address with path
String openWeatherMapApiKey = "a8d291fce961ee20987d3d6f961bec58";

// Replace with your country code and city
String city = "Seoul";
String countryCode = "KR";

// Timer set to 15 seconds for demonstration
unsigned long lastTime = 0;
unsigned long timerDelay = 15000;

String jsonBuffer;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Timer set to 15 seconds (timerDelay variable), it will take 15 seconds before publishing the first reading.");
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    if (WiFi.status() == WL_CONNECTED) {
      String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey;
      jsonBuffer = httpGETRequest(serverPath.c_str());
      JSONVar myObject = JSON.parse(jsonBuffer);

      if (JSON.typeof(myObject) == "undefined") {
        Serial.println("Parsing input failed!");
        return;
      }

      Serial.print("Latitude: ");
      Serial.println(myObject["coord"]["lat"]);
      Serial.print("Longitude: ");
      Serial.println(myObject["coord"]["lon"]);
      
      Serial.println("Weather Details:");
      for (int i = 0; i < myObject["weather"].length(); i++) {
        Serial.print("  Weather ID: ");
        Serial.println(myObject["weather"][i]["id"]);
        Serial.print("  Main: ");
        Serial.println(myObject["weather"][i]["main"]);
        Serial.print("  Description: ");
        Serial.println(myObject["weather"][i]["description"]);
        Serial.print("  Icon: ");
        Serial.println(myObject["weather"][i]["icon"]);
      }

      Serial.print("Temperature: ");
      Serial.println(myObject["main"]["temp"]);
      Serial.print("Feels Like: ");
      Serial.println(myObject["main"]["feels_like"]);
      Serial.print("Minimum Temperature: ");
      Serial.println(myObject["main"]["temp_min"]);
      Serial.print("Maximum Temperature: ");
      Serial.println(myObject["main"]["temp_max"]);
      Serial.print("Pressure: ");
      Serial.println(myObject["main"]["pressure"]);
      Serial.print("Humidity: ");
      Serial.println(myObject["main"]["humidity"]);

      Serial.print("Wind Speed: ");
      Serial.println(myObject["wind"]["speed"]);
      Serial.print("Wind Direction (Degrees): ");
      Serial.println(myObject["wind"]["deg"]);
      if (myObject["wind"].hasOwnProperty("gust")) {
        Serial.print("Wind Gust: ");
        Serial.println(myObject["wind"]["gust"]);
      }
      
      if (myObject.hasOwnProperty("rain")) {
        Serial.print("Rain Volume for last hour: ");
        Serial.println(myObject["rain"]["1h"]);
      }
      
      if (myObject.hasOwnProperty("snow")) {
        Serial.print("Snow Volume for last hour: ");
        Serial.println(myObject["snow"]["1h"]);
      }

      Serial.print("Visibility: ");
      Serial.println(myObject["visibility"]);
      
      Serial.print("Cloudiness (%): ");
      Serial.println(myObject["clouds"]["all"]);

      lastTime = millis();
    } else {
      Serial.println("WiFi Disconnected");
    }
  }
}

String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
  http.begin(client, serverName);
  int httpResponseCode = http.GET();
  String payload = "{}"; 

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
  return payload;
}