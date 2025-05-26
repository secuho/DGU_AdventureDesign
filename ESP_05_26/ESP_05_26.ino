#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define TXD1 19
#define RXD1 21
#define code_digits 12//11글자의 문자열에 날씨 정보를 포함해서 전달할 것임, 필요에 따라 글자수는 늘리거나 줄일 수 있음

HardwareSerial arduSerial(1);

char data[code_digits];

const char* ssid = "yourssid";
const char* password = "yourpasswd";

float rain_sum = 0;
float temp_sum = 0;
bool temp_sign = 0;//기온의 부호, 0이면 +, 1이면 -
int rain_code = -1;
int weather_id = 800;//3자릿수의 날씨 id, 800(맑음)이 아닌 가장 먼저 다가오는 날씨 중, 어떤 상황이 있는지 알려줌
bool has_checked_first_not_800_code = 0;

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
        int id = forecast["weather"][0]["id"];
        float rain = forecast["rain"]["1h"] | 0.0;

        if(i>=8 && i<=31)
        {
          rain_sum+=rain;
          temp_sum+=temp;
          if(id!=800 && has_checked_first_not_800_code == 0)
          {
            weather_id = id;
            has_checked_first_not_800_code = 1;
          }
        }
        // 출력
        Serial.println("=== 날씨 예보 ===");
        Serial.print("시간: "); Serial.println(time);
        Serial.print("기온: "); Serial.print(temp); Serial.println(" °C");
        Serial.print("습도: "); Serial.print(humidity); Serial.println(" %");
        Serial.print("날씨: "); Serial.println(weatherMain);
        Serial.print("설명: "); Serial.println(description);
        Serial.print("1시간 강수량: "); Serial.print(rain); Serial.println(" mm");
        Serial.print("날씨 코드: "); Serial.println(id);
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
      if(temp_sum<0)
      temp_sign = 1;

      data[0] = '@';//시작 문자
      data[1] = '0' + rain_code;//비가 얼마나 강하게 오는지 코드 
      data[2] = '0' + (int)(rain_sum / 24) / 10;//일평균 강수량의 십의 자리 
      data[3] = '0' + (int)(rain_sum / 24) % 10;//일평균 강수량의 일의 자리
      data[4] = '0' + temp_sign;//일평균 기온의 부호
      data[5] = '0' + (int)(temp_sum / 24) / 10;//일평균 기온의 십의 자리
      data[6] = '0' + (int)(temp_sum / 24) % 10;//일평균 기온의 일의 자리
      data[7] = '0' + weather_id / 100;//날씨 코드 백의 자리
      data[8] = '0' + (weather_id % 100) / 10;//날씨 코드 십의 자리
      data[9] = '0' + weather_id % 10;//날씨 코드 일의 자리
      data[10] = '#';//weather_code상의 종료문자
      data[11] = 0;//C++ 문법상의 종료 문자
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
  rain_sum = 0;
  temp_sum = 0;
  temp_sign = 0;
  has_checked_first_not_800_code = 0;
  // 60초 대기
  for(int i=20;i>=0;i--)
  {
    delay(1000);
    Serial.print(i);Serial.println("초 후 다음 날씨 호출...");
  }
}