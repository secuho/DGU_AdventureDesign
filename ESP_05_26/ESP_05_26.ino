#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define TXD1 19
#define RXD1 21

HardwareSerial arduSerial(1);

char data[6];

const char* ssid = "yourssid";
const char* password = "yourpasswd";

float rain_sum = 0;
int rain_code = -1;

const char* weatherUrl = "https://pro.openweathermap.org/data/2.5/forecast/hourly?q=Seoul,KR&appid=a8d291fce961ee20987d3d6f961bec58&units=metric";


void setup() {
  Serial.begin(115200);
  delay(1000);

  //Wi-Fi 연결
  WiFi.begin(ssid, password);
  Serial.print("Wi-Fi 연결 중");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi 연결 완료!");

  arduSerial.begin(9600, SERIAL_8N1, RXD1, TXD1);
  
  Serial.println("ESP32 UART Transmitter");
}

void loop() {
  if ((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;
    http.begin(weatherUrl);
    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();

      // JSON 파싱
      DynamicJsonDocument doc(8192); // 크기 조절 가능
      DeserializationError error = deserializeJson(doc, payload);

      if (error) {
        Serial.print("JSON 파싱 오류: ");
        Serial.println(error.c_str());
      } else {
        // 첫 번째 예보 추출
        for(int i=0;i<doc["list"].size();i++)
        {
        Serial.print(i);
        Serial.println("번째 예보");

        JsonObject forecast = doc["list"][i];

        const char* time = forecast["dt_txt"];
        float temp = forecast["main"]["temp"];
        int humidity = forecast["main"]["humidity"];
        const char* weatherMain = forecast["weather"][0]["main"];
        const char* description = forecast["weather"][0]["description"];
        float rain = forecast["rain"]["1h"] | 0.0;

        if(i>=8 && i<=31)
        {
          rain_sum+=rain;
        }
        // 출력
        Serial.println("=== 날씨 예보 ===");
        Serial.print("시간: "); Serial.println(time);
        Serial.print("기온: "); Serial.print(temp); Serial.println(" °C");
        Serial.print("습도: "); Serial.print(humidity); Serial.println(" %");
        Serial.print("날씨: "); Serial.println(weatherMain);
        Serial.print("설명: "); Serial.println(description);
        Serial.print("1시간 강수량: "); Serial.print(rain); Serial.println(" mm");
        Serial.println("=================\n");
        }
        Serial.println("============================================\n");
        Serial.print("향후 24시간 강수량 총합: ");Serial.println(rain_sum);

        if(rain_sum == 0)
          rain_code = 0;//비 없음
        else if(rain_sum<72)
          rain_code = 1;//약한 비
        else if(rain_sum<360)
          rain_code = 2;//보통 비
        else
          rain_code = 3;//강한 비

        Serial.print("PREP: ");Serial.print(rain_code);
        switch(rain_code)
        {
          case 0:
            Serial.println(" ---> 비가 없습니다.");
            break;
          case 1:
            Serial.println(" ---> 약한 비가 올 예정입니다.");
            break;
          case 2:
            Serial.println(" ---> 보통 비가 올 예정입니다.");
            break;
          case 3:
            Serial.println(" ---> 강한 비가 올 예정입니다.");
            break;
          default:
            Serial.println(" ---> RAIN COUNT ERROR!!!");
        }
        Serial.println("============================================\n");
      }
      data[0] = 'H';
      data[1] = 'e';
      data[2] = 'l';
      data[3] = 'l';
      data[4] = 'o';
      data[5] = 0;
      arduSerial.write(data);//ESP->아두이노 data 전송
      Serial.print("Data Sended : ");
      Serial.println(data);

    } else {
      Serial.print("HTTP 요청 실패, 코드: ");
      Serial.println(httpCode);
    }

    http.end();
  } else {
    Serial.println("Wi-Fi 연결이 끊겼습니다.");
  }
  // 60초 대기
  for(int i=20;i>=0;i--)
  {
    delay(1000);
    Serial.print(i);Serial.println("초 후 다음 날씨 호출...");
  }
}