#include <WiFi.h>
#include <WebServer.h>

const char *ssid = "ESP32-AP";
const char *password = "123456789";

#define LED_PIN           14
#define Led1On()          digitalWrite(LED_PIN, 0) 
#define Led1Off()         digitalWrite(LED_PIN, 1)

#define SW_PIN            1

WebServer server(80);  // Object of WebServer(HTTP port, 80 is defult)
void handle_root();

int motor_step = 0;

// HTML 페이지
#if 1
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html> <html>
<head><meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
<title>LED Control</title>
	<style>
body{
  background:#2c3e50;
  font-family: 'Open Sans', sans-serif;
}

h1, button{
  color:#fff;
  text-align: center;
  padding: 20px;
}

p{
  color:#fff;
  text-align: center;
  padding-top: 500px;
  font-size: 10px;
}

a{
   text-decoration:none;
   color:#fff;
}
.main{
  text-align: center;
  width: 100%;
  margin-bottom:20px;
}

.button-1{
  text-align: center;
  cursor: pointer;
  font-size:24px;
}

.button-1 {
  border-radius: 10px;
  //background-color:#d32000;
  background-color: #27ae60;
  border: none;
  padding: 20px;
  width: 300px;
  box-shadow: 0 2px #FFFFFF;
}
</style>
</head>
<body>
  <h1>Simple Webserber</h1>
  <div class="main">
      <a href="on"><button class="button-1"><span>LED On</span></button></a>
  </div>
  <div class="main">
      <a href="off"><button class="button-1"><span>LED Off</span></button></a>
  </div>  
</body>
)rawliteral";
#endif


//페이지 요청이 들어 오면 처리 하는 함수
void handle_root()
{
  server.send(200, "text/html", index_html);
}


//On 버튼 페이지 처리함수
void SetLedStatusOn()
{
    Serial.println("On");
    Led1On();
    server.send(200, "text/html", index_html);
}

//Off 버튼 페이지 처리함수
void SetLedStatusOff()
{
    Serial.println("Off");
    Led1Off();
    server.send(200, "text/html", index_html);
}


void InitWiFi(void)
{
  WiFi.mode(WIFI_AP); // Only Access point (AP mode)
  IPAddress myIP = WiFi.softAP(ssid, password);

  Serial.print("AP SSID: ");
  Serial.println(ssid);
  Serial.println("HTTP server started");
 
  Serial.print("HotSpt IP:");
  Serial.println(myIP);
}

void InitWebServer() 
{
  //페이지 요청 처리 함수 등록
  server.on("/", handle_root);
  server.on("/index.html", handle_root);
  server.on("/on", SetLedStatusOn);
  server.on("/off", SetLedStatusOff);
  server.begin();
}


void setup() {
    pinMode(LED_PIN, OUTPUT);
    Led1On();   
    pinMode(SW_PIN, INPUT_PULLUP);

    Serial.begin(115200);
    Serial.println("ESP32 Simple web Start");
    Serial.println(ssid);

    InitWiFi();
    InitWebServer(); 

    Serial.println("HTTP server started");
    delay(100); 
}

void loop() {
  server.handleClient();
}