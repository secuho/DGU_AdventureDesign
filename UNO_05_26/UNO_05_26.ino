#include <SoftwareSerial.h>
#define code_digits 12//11글자의 문자열에 날씨 정보를 포함해서 전달할 것임, 필요에 따라 글자수는 늘리거나 줄일 수 있음

SoftwareSerial espSerial(2, 3); // RX, TX

char recent_data;
char weather_code[code_digits];
int wc_index = 0;
bool is_today_rainy = 0;

void setup() {
  Serial.begin(115200);
  espSerial.begin(9600);

  Serial.println("Arduino UART Receiver");
}

//날씨 코드 설명
//weather_code[0] : @를 고정적으로 입력받아, 문자열의 시작을 알리는 공간
//weather_code[1] : 그날의 날씨를 간단하게 표현할 수 있는 코드
//weather_code[2] : 비가 온다면, 24시간 강수량총합/24를 통해 하루 평균 강수량을 받는 곳, 하루 평균 강수량의 십의 자리.
//weather_code[3] :  비가 온다면, 24시간 강수량총합/24를 통해 하루 평균 강수량을 받는 곳, 하루 평균 강수량의 일의 자리.
//weather_code[4] : 일평균 기온의 부호
//weather_code[5] : 일평균 기온의 십의 자리
//weather_code[6] : 일평균 기온의 일의 자리
//weather_code[7] : 일단 무기능
//weather_code[8] : 일단 무기능
//weather_code[9] : #을 고정적으로 입력받아, 문자열의 종료를 알리는 공간. #을 입력받으면 시리얼에서 제대로 입력받았는지 방금 입력받은 weather_code string을 확인차 출력시켜주자

void loop() {
  if (espSerial.available())
  {
    recent_data = espSerial.read();
    Serial.print("Received: ");
    Serial.println(recent_data);

    if(recent_data == '@')//문자 '@'를 받으면 입력이 시작된 것으로 간주한다.
    {
      wc_index = 0;
    }

    weather_code[wc_index] = recent_data;
    wc_index++;
    
    if(recent_data == '#')//문자 '#'을 받으면 입력이 종료된 것으로 간주하고, weather_code를 확인차 한 번 출력한다.
    {
      weather_code[code_digits] = 0;
      Serial.print("weather_code: ");
      Serial.println(weather_code);
    }
  }
}