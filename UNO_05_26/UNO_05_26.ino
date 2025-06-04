#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>

#define code_digits 12//11글자의 문자열에 날씨 정보를 포함해서 전달할 것임, 필요에 따라 글자수는 늘리거나 줄일 수 있음

SoftwareSerial espSerial(2, 3); // RX, TX->UNO통신용
LiquidCrystal_I2C lcd(0x27, 20, 4);

SoftwareSerial mySoftwareSerial(10, 11); // RX, TX->오디오 재생용
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

char recent_data;
char weather_code[code_digits];
int wc_index = 0;
bool is_today_rainy = 0;
bool is_user_in_the_front_door = 0;
bool is_umbrella_hooked = 0;
int rain_sum = -999;
int temperature = -999;
int weather_id = -999;

char track[1];

void setup() {
  //시리얼 수신 준비
  Serial.begin(115200);
  espSerial.begin(9600);
  Serial.println("Arduino UART Receiver");
  ////////////////////////////////////////////////
  //LCD 출력 준비
  lcd.init();
  lcd.backlight();//백라이트 켬
  lcd.setCursor(0,0);
  lcd.print("Weather Auto Sys!");
  lcd.setCursor(0,1);
  lcd.print("Hello World!");
  ////////////////////////////////////////////////
  //오디오 출력 준비
  mySoftwareSerial.begin(9600);
  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true){
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }
  Serial.println(F("DFPlayer Mini online."));
  
  myDFPlayer.volume(20);  //Set volume value. From 0 to 30
  ///////////////////////////////////////////////////
}

//날씨 코드 설명
//weather_code[0] : @를 고정적으로 입력받아, 문자열의 시작을 알리는 공간
//weather_code[1] : 그날의 날씨를 간단하게 표현할 수 있는 코드
//weather_code[2] : 비가 온다면, 24시간 강수량총합/24를 통해 하루 평균 강수량을 받는 곳, 하루 평균 강수량의 십의 자리.
//weather_code[3] :  비가 온다면, 24시간 강수량총합/24를 통해 하루 평균 강수량을 받는 곳, 하루 평균 강수량의 일의 자리.
//weather_code[4] : 일평균 기온의 부호
//weather_code[5] : 일평균 기온의 십의 자리
//weather_code[6] : 일평균 기온의 일의 자리
//weather_code[7] : 날씨 코드 백의 자리
//weather_code[8] : 날씨 코드 십의 자리
//weather_code[9] : 날씨 코드 일의 자리
//weather_code[10] : #을 고정적으로 입력받아, 문자열의 종료를 알리는 공간. #을 입력받으면 시리얼에서 제대로 입력받았는지 방금 입력받은 weather_code string을 확인차 출력시켜주자

void loop() {
  _esp_communication();
  _umbrella_control();
  _pir_sensing();
}
void _pir_sensing()
{
  /*if(pir이 인식된다면)
  {
    is_user_in_the_front_door = 1;
  }
  */
}
void _umbrella_control()
{
  if(is_user_in_the_front_door == 1 && is_today_rainy == 1 && is_umbrella_hooked == 1)
  {
    //외출하는 상황, 비가 오는 상황
    //로봇팔 내밀어 우산 건네주기
    //날씨 알려주기
    _playing_weather_audio_and_lcd_print();
    is_umbrella__hooked = 0;
  }
  else if(is_user_in_the_front_door == 1 && is_umbrella_hooked == 1)
  {
    //외출하는 상황, 비는 안오는 상황
    //날씨 알려주기
    _playing_weather_audio_and_lcd_print();
  }
  else if(is_user_in_the_front_door == 1 && is_umbrella_hooked == 0)
  {
    //귀가한 상황
    //로봇팔 내밀어 우산 보관할 준비하기
    is_umbrella__hooked = 1;
  }
}

void _esp_communication()
{
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
      /////////////////////////////////////////////////////////////
      //입력 종료 후 수신한 정보에 따라 정보 가공
      //강수량이 0보다 크면 비가 오는 것으로 간주
      rain_sum = (weather_code[2] - '0') * 10 + (weather_code[3] - '0');
      if(rain_sum == 0)
      {
        is_today_rainy = 0;
      }
      else
      {
        is_today_rainy = 1;
      }
      //평균 온도 가공
      temperature = (weather_code[5] - '0') * 10 + (weather_code[6] - '0');
      if(weather_code[4] - '0' == 1)//부호 비트가 1이라면: 기온이 영하라면
      {
        temperature *= -1;
      }
      //날씨 id 가공
      weather_id = (weather_code[7] - '0') * 100 + (weather_code[8] - '0') * 10 + (weather_code[9] - '0');
      //////////////////////////////////////////////////////////////
      Serial.print("rain_sum: ");Serial.println(rain_sum);
      Serial.print("is_today_rainy: ");Serial.println(is_today_rainy);
      Serial.print("temperature: ");Serial.println(temperature);
      Serial.print("weather_id: ");Serial.println(weather_id);
      _playing_weather_audio_and_lcd_print();
    }
  }
}
void _playing_weather_audio_and_lcd_print()
{
  //가공한 weather_id에 따라서 재생할 오디오를 고르고 디스플레이에 출력하는 함수
  switch(weather_id)
  {
    case 200:
    case 201:
    case 202:
    case 210:
    case 211:
    case 212:
    case 221:
    case 230:
    case 231:
    case 232:
      //오늘의 날씨는 천둥번개를 동반한 비바람입니다
      lcdprint(0);
      myDFPlayer.play(11);
      break;
    case 300:
    case 301:
    case 302:
    case 310:
    case 311:
    case 312:
    case 313:
    case 314:
    case 321:
      //오늘의 날씨는 이슬비입니다
      lcdprint(1);
      myDFPlayer.play(10);
      break;
    case 500:
    case 501:
    case 502:
    case 503:
    case 504:
    case 511:
    case 520:
    case 521:
    case 522:
    case 531:
      //오늘의 날씨는 비입니다 우산을 가져가세요
      lcdprint(2);
      myDFPlayer.play(9);
      break;
    case 600:
    case 601:
    case 602:
    case 611:
    case 612:
    case 613:
    case 615:
    case 616:
    case 620:
    case 621:
    case 622:
      //오늘의 날씨는 눈입니다 우산을 가져가세요
      lcd.print(3);
      myDFPlayer.play(7);
      break;
    case 701:
    case 711:
    case 721:
    case 741:
      //오늘의 날씨는 안개가 많이 끼어있습니다
      lcdprint(4);
      myDFPlayer.play(1);
      break;
    case 731:
    case 751:
    case 761:
      //오늘의 날씨는 황사가 심합니다 마스크를 쓰세요
      lcdprint(5);
      myDFPlayer.play(5);
      break;
    case 762:
      //오늘의 날씨는 화산재가 날리는 날입니다 마스크를 쓰세요
      lcdprint(6);
      myDFPlayer.play(4);
      break;
    case 771:
      //오늘은 열대 지방의 스콜 소나기가 올지도 모릅니다 우산을 가져가세요
      lcdprint(7);
      myDFPlayer.play(2);
      break;
    case 781:
      //오늘은 토네이도가 발생할지도 모릅니다 바깥 활동에 주의하세요
      lcdprint(8);
      myDFPlayer.play(3);//오디오 이슈로 일단 3번 대신 11번으로 재생
      break;
    case 800:
      //오늘의 날씨는 맑음입니다
      lcdprint(9);
      myDFPlayer.play(8);
      break;
    case 801:
    case 802:
    case 803:
    case 804:
    //오늘의 날씨는 구름과 함께 맑음입니다
      lcdprint(10);
      myDFPlayer.play(6);
      break;
    default:
      //weather_id error
      break;
  }
}
void lcdprint(int menu)
{
  switch(menu)
  {
    case 0:
      lcd.setCursor(0,0);
      lcd.print("                ");
      lcd.setCursor(0,0);
      lcd.print("Thunderstorm!");
      lcd.setCursor(0,1);
      lcd.print("                ");
      lcd.setCursor(0,1);
      lcd.print("Be careful!");
      break;
    case 1:
      lcd.setCursor(0,0);
      lcd.print("                ");
      lcd.setCursor(0,0);
      lcd.print("Drizzle!");
      lcd.setCursor(0,1);
      lcd.print("                ");
      lcd.setCursor(0,1);
      lcd.print("Take your umbrella!");
      break;
    case 2:
      lcd.setCursor(0,0);
      lcd.print("                ");
      lcd.setCursor(0,0);
      lcd.print("Today's Rain!");
      lcd.setCursor(0,1);
      lcd.print("                ");
      lcd.setCursor(0,1);
      lcd.print("Take your umbrella!");
      break;
    case 3:
      lcd.setCursor(0,0);
      lcd.print("                ");
      lcd.setCursor(0,0);
      lcd.print("Today's Snow!");
      lcd.setCursor(0,1);
      lcd.print("                ");
      lcd.setCursor(0,1);
      lcd.print("Take your umbrella!");
      break;
    case 4:
      lcd.setCursor(0,0);
      lcd.print("                ");
      lcd.setCursor(0,0);
      lcd.print("Today's Foggy!");
      lcd.setCursor(0,1);
      lcd.print("                ");
      lcd.setCursor(0,1);
      lcd.print("Be careful of sight");
      break;
    case 5:
      lcd.setCursor(0,0);
      lcd.print("                ");
      lcd.setCursor(0,0);
      lcd.print("Today's Dust!");
      lcd.setCursor(0,1);
      lcd.print("                ");
      lcd.setCursor(0,1);
      lcd.print("Take your mask!");
      break;
    case 6:
      lcd.setCursor(0,0);
      lcd.print("                ");
      lcd.setCursor(0,0);
      lcd.print("Today's Volcanic Ash!");
      lcd.setCursor(0,1);
      lcd.print("                ");
      lcd.setCursor(0,1);
      lcd.print("Take your mask!");
      break;
    case 7:
      lcd.setCursor(0,0);
      lcd.print("                ");
      lcd.setCursor(0,0);
      lcd.print("Today's Squall!");
      lcd.setCursor(0,1);
      lcd.print("                ");
      lcd.setCursor(0,1);
      lcd.print("Take your umbrella!");
      break;
    case 8:
      lcd.setCursor(0,0);
      lcd.print("                ");
      lcd.setCursor(0,0);
      lcd.print("Today's Tornado!");
      lcd.setCursor(0,1);
      lcd.print("                ");
      lcd.setCursor(0,1);
      lcd.print("Don't go outside!");
      break;
    case 9:
      lcd.setCursor(0,0);
      lcd.print("                ");
      lcd.setCursor(0,0);
      lcd.print("Today's Clear!");
      lcd.setCursor(0,1);
      lcd.print("                ");
      lcd.setCursor(0,1);
      lcd.print("Have a nice day!");
      break;
    case 10:
      lcd.setCursor(0,0);
      lcd.print("                ");
      lcd.setCursor(0,0);
      lcd.print("Today's Clouds!");
      lcd.setCursor(0,1);
      lcd.print("                ");
      lcd.setCursor(0,1);
      lcd.print("Have a nice day!");
      break;
  }
}