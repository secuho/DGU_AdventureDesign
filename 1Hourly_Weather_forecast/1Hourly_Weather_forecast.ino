#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Wi-Fi 정보 입력
const char* ssid = "wifiID";
const char* password = "wifiPASSWD";

// OpenWeatherMap API
const char* weatherUrl = "https://pro.openweathermap.org/data/2.5/forecast/hourly?q=Seoul,KR&appid=a8d291fce961ee20987d3d6f961bec58&units=metric";

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Wi-Fi 연결
  WiFi.begin(ssid, password);
  Serial.print("Wi-Fi 연결 중");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi 연결 완료!");
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
        Serial.println("============================================\n");
      }
    } else {
      Serial.print("HTTP 요청 실패, 코드: ");
      Serial.println(httpCode);
    }

    http.end();
  } else {
    Serial.println("Wi-Fi 연결이 끊겼습니다.");
  }

  delay(60000);  // 60초 대기
}